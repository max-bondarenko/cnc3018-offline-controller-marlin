#include "JobFsm.h"
#include "etl/string_utilities.h"
#include "constants.h"

bool JobFsm::readCommandsToBuffer() {
    GCodeDevice& _dev = *dev;

    char curLine[MAX_LINE_LEN];
    if (filePos >= fileSize) {
        return false;
    }

    if (resendLineNum != 0xFF) {
        auto tail = cmdBuffer.tail((uint8_t) (readLineNum - resendLineNum ));
        auto end = cmdBuffer.end();

        while (!_dev.buffer.full()) {
            if (tail != end) {
                const CmdInFile& cmd = *tail;
                gcodeFile.seek(cmd.position);
                gcodeFile.read(curLine, cmd.length);
                String line;
                getString(line, curLine, resendLineNum);
                if (!_dev.scheduleCommand(line.c_str(), line.length())) {
                    break;
                }
                resendLineNum++;
                ++tail;
            } else {
                resendLineNum = 0xFF;
                break;
            }
        }

    } else {
        uint16_t curLinePos = 0;
        size_t begin = filePos;

        //usually gcode has no long empty regions
        for (size_t gard = 10; gard > 0; --gard) { // loop gard
            bool got_comment = false;
            while (gcodeFile.available() > 0) {
                int readChar = gcodeFile.read();
                filePos++;
                if (';' == readChar || '(' == readChar) {
                    got_comment = true;
                    continue;
                }
                if ('\n' == readChar || '\r' == readChar) {
                    got_comment = false;
                    if (curLinePos != 0)
                        break; // if it's an empty string or LF after last CR, just continue reading
                } else {
                    if (curLinePos < MAX_LINE_LEN && !got_comment) {
                        curLine[curLinePos++] = readChar;
                    }
                }
            }
            curLine[curLinePos] = 0;
            bool empty = true;
            for (size_t i = 0; i < curLinePos; i++) {
                if (!isspace(curLine[i])) {
                    empty = false;
                }
            }
            if (curLinePos == 0 || empty) {
                JOB_LOGLN("Empty line");
                curLinePos = 0;
            } else
                break;
        }
        if (curLinePos == 0) {
            JOB_LOGLN("end of file");
            return true;
        }

        char* end = curLine + curLinePos - 1;
        while (isspace(*end) && end >= curLine) {
            end--;
            *end = 0;
        }
        String line;
        getString(line, curLine, readLineNum);
        _dev.scheduleCommand(line.c_str(), line.length());
        cmdBuffer.push(CmdInFile{begin, curLinePos, readLineNum});
        readLineNum++;
        --cmdBuffer;
    }


    return true;
}

void JobFsm::getString(String& line, const char* curLine, size_t lineNumber) {
    if (addLineN) {
        // line number and checksum
        line = "N";
        line += String(lineNumber);
        line += " ";
        line += curLine;
        line.trim();
        uint8_t checksum = calculateChecksum(line.c_str(), line.length());
        line += '*';
        line += StringSumHelper(checksum);
    } else {
        line = curLine;
    }
}

uint8_t JobFsm::calculateChecksum(const char* out, uint8_t count) const {
    uint8_t checksum = 0;
    while (count)
        checksum ^= out[--count];
    return checksum;
}

void JobFsm::setFile(const char* file) {
    gcodeFile = SD.open(file);
    if (gcodeFile) {
        fileSize = gcodeFile.size();
    }
    filePos = 0;
    readLineNum = 0;
    cmdBuffer.clear();
    startTime = 0;
    endTime = 0;
}

uint32_t JobFsm::getPrintDuration() const {
    return (endTime != 0 ? endTime : millis()) - startTime;
}

void JobFsm::closeFile() {
    if (gcodeFile) {
        gcodeFile.close();
    }
}

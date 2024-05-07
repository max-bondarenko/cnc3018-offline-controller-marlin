#include "JobFsm.h"
#include "constants.h"
#include "WString.h"

bool JobFsm::readCommandsToBuffer() {
    GCodeDevice& _dev = *dev;
    char curLine[MAX_LINE_LEN];
    if (resendLineNum != 0xFFFF) {
        auto tail = cmdBuffer.tail((uint8_t) (readLineNum - resendLineNum));
        auto end = cmdBuffer.end();

        while (dev->canSchedule()) {
            if (tail != end) {
                const CmdInFile& cmd = *tail;
                gcodeFile.seek(cmd.position);
                gcodeFile.read(curLine, cmd.length);
                curLine[cmd.length] = 0;
                String line;
                buildCommand(line, curLine, resendLineNum, addLineN);
                _dev.scheduleCommand(line.c_str(), line.length());
                resendLineNum++;
                ++tail;
            } else {
                resendLineNum = 0xFFFF;
                gcodeFile.seek(filePos - 1);
                break;
            }
        }
    } else {
        while (dev->canSchedule()) {
            uint16_t curLinePos = 0;
            size_t begin;
            size_t end = 0;
            bool comment = false;
            int av;
            while ((av = gcodeFile.available()) > 0) {
                char readChar = gcodeFile.read();
                filePos++;
                end++;
                if (readChar == '\n') {
                    break;
                }
                if (comment)
                    continue;
                if (readChar == ';' || readChar == '(') {
                    comment = true;
                    continue;
                }
                curLine[curLinePos++] = readChar;
            }
            if (av == 0) {
                return false;
            }
            curLine[curLinePos] = 0;
            begin = filePos - end;
            {
                bool empty = true;
                for (size_t j = 0; j < curLinePos; j++) {
                    if (!isspace(curLine[j])) {
                        empty = false;
                    }
                }
                if (curLinePos == 0 || empty) {
                    continue;
                }
            }
            char* _end = curLine + curLinePos - 1;
            while (isspace(*_end) && _end >= curLine) {
                *_end = 0;
                curLinePos--;
                _end--;
            }
            JOB_LOGF(">place #%d_[%s] at:%d l:%d, fpos:%d\n", readLineNum, curLine, begin, curLinePos, filePos);
            String line;
            buildCommand(line, curLine, readLineNum, addLineN);
            cmdBuffer.push(CmdInFile{begin, (uint8_t) curLinePos});
            readLineNum++;
            _dev.scheduleCommand(line.c_str(), line.length());
            --cmdBuffer;
        }
    }
    return true;
}

void JobFsm::buildCommand(String& line, const char* curLine, size_t lineNumber, bool addLineN) {
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

uint8_t JobFsm::calculateChecksum(const char* out, uint8_t count) {
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

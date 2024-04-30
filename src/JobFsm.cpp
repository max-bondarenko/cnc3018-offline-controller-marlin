#include "JobFsm.h"
#include "WString.h"
#include "constants.h"

bool JobFsm::readCommandsToBuffer() {
    if (filePos >= fileSize) {
        return false;
    }
    for (size_t i = MAX_READ_CHUNK; i != 0; i--) {
        size_t curLinePos = 0;
        if (readLineNum - currentLineNum > MAX_READ_CHUNK - 1) {
            break;
        }
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
        String* l = new String(curLine);
        {
            String& line = (*l);
            if (addLineN) {
                // line number and checksum
                line = "N";
                line += String(readLineNum);
                line += " ";
                line += curLine;
                line.trim();
                uint8_t checksum = 0, count = line.length();
                const char* out = line.c_str();
                while (count)
                    checksum ^= out[--count];
                line += '*';
                line += String(checksum);
            } else {
                line = curLine;
            }
        }

        int j = readLineNum % JOB_BUFFER_SIZE;
        JOB_LOGF("put [%s] at %d \n", l->c_str(), j);
        String* string = buffer[j];
        if (string != nullptr)
            string->~String();

        buffer[j]= l;
        ++readLineNum;
    }
    return true;
}

void JobFsm::setFile(const char* file) {
    gcodeFile = SD.open(file);
    if (gcodeFile) {
        fileSize = gcodeFile.size();
    }
    filePos = 0;
    readLineNum = 0;
    currentLineNum = 0;
    for (int i = 0; i < JOB_BUFFER_SIZE; ++i) {
        if (buffer[i])
            buffer[i]->~String();
    }

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

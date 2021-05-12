#include "Process.h"

namespace util {
    bool SubProcess::readLineFromBuffer(std::string& line) const {
        if (m_bufferLoc <= 0) {
            return false;
        }
        auto bufferEnd = readBuffer.begin() + m_bufferLoc + 1;
        auto newLine = std::find(readBuffer.begin(), bufferEnd, '\n');

        if (newLine == bufferEnd) {
            return false;
        }

        // include newline in message
        ++newLine;

        line = std::string(readBuffer.begin(), newLine);

        std::fill(readBuffer.begin(), newLine, '\0');

        // optimization if newLine was the last real character skip rotating
        if (std::distance(readBuffer.begin(), newLine) <= m_bufferLoc) {
            std::rotate(readBuffer.begin(), newLine, readBuffer.end());
            m_bufferLoc -= std::distance(readBuffer.begin(), newLine);
        } else {
            m_bufferLoc = 0;
        }


        return true;
    }
}

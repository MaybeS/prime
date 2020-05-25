//
// Created by Jiun Bae on 2020/05/25.
//

#ifndef PROGRESS_HPP
#define PROGRESS_HPP

#include <ostream>

class Progress {
    public:
        float start, end, now;
        float step;
        int width;

        explicit Progress(float begin = 0.f, float step = 0.f,
                float start = 0.f, float end = 100.f, int width = 70)
        : now(begin), step(step), start(start), end(end), width(width) {
        }

        void update(float value = 1.f) {
            if (step != .0f) {
                value = this->step;
            }
            this->now += value;
        }

        void clear() {
            this->now = 0.f;
        }

        std::ostream& operator<<(std::ostream& ostream) const {
            return ostream;
        }
};

std::ostream& operator<<(std::ostream& ostream, const Progress& progress) {
    ostream << "[";
    int pos = static_cast<int>(static_cast<float>(progress.width) * progress.now / 100.f);
    for (int i = 0; i < progress.width; ++i) {
        if (i < pos) ostream << "=";
        else if (i == pos) ostream << ">";
        else ostream << " ";
    }
    ostream << "] " << static_cast<int>(progress.now) << " %\r";
    ostream.flush();

    return ostream;
}

#endif //PROGRESS_HPP

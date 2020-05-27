//
// Created by Jiun Bae on 2020/05/25.
//

#ifndef PROGRESS_HPP
#define PROGRESS_HPP

#include <ostream>

class Progress {
    public:
        const char* title;
        float start, end, now;
        float step;
        size_t width;

        explicit Progress(const char* title="", float begin=0.f, float step=0.f,
                float start=0.f, float end=100.f, size_t width=70)
        : title(title), now(begin), step(step),
        start(start), end(end), width(width) {}

        void update(float value = 1.f) {
            if (step != .0f) {
                value = this->step;
            }
            this->now += value;
        }

        void clear(float value=0.f) {
            this->now = value;
        }

        std::ostream& operator<<(std::ostream& ostream) const {
            return ostream;
        }
};

std::ostream& operator<<(std::ostream& ostream, const Progress& progress) {

    ostream << progress.title << " [";
    auto pos = static_cast<size_t>(static_cast<float>(progress.width) * progress.now / 100.f);
    for (size_t i = 0; i < progress.width; ++i) {
        if (i < pos) {
            ostream << "=";
        } else if (i == pos) {
            ostream << ">";
        } else {
            ostream << " ";
        }
    }
    ostream << "] " << static_cast<size_t>(progress.now) << " %\r";
    ostream.flush();

    return ostream;
}

#endif //PROGRESS_HPP

#ifndef BEAT_SIMULATOR_H
#define BEAT_SIMULATOR_H

#include "animations.h"
#include "playback.h"

class BeatSimulator
{
    public:
        const int BEAT_IN_ROW = 8;
        const int BEAT_PAUSE = 4;

        Animations *animations;
        float BPM;
        float beatDurationMs;
        long nextBeatMs;
        int beatCount;
        bool isInPause;

        BeatSimulator(Animations *animations, float BPM)
        {
            this->animations = animations;
            this->BPM = BPM;
            beatDurationMs = 1000 * 60 / BPM;
            nextBeatMs = getMs() + beatDurationMs * 2;
            beatCount = BEAT_IN_ROW;
        }

        void update()
        {
            unsigned long ms = getMs();
            if (ms >= nextBeatMs)
            {
                nextBeatMs += beatDurationMs;
                beatCount--;
                if (beatCount <= 0)
                {
                    isInPause = !isInPause;
                    beatCount = isInPause ? BEAT_PAUSE : BEAT_IN_ROW;
                }

                if (!isInPause)
                {
                    animations->beat();
                }
            }
        }
};

#endif
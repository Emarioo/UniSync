#pragma once

#include "Engone/Utilities/Utilities.h"

namespace engone {
	struct RuntimeStats {
		enum PrintFlag : uint8_t {
			PrintSamples=1, // fps, ups
			PrintFramesUpdates=2, // total frames and updates
			PrintEngineTime =4, // time since start and sleep time
		};
		typedef uint8_t PrintFlags;
		RuntimeStats() { setFPSLimit(60); setUPSLimit(60); }

		// frames per second, 1-999
		// Use applyNewLimits to apply changes. (IS DONE IN THE ENGINE LOOP. DO NOT DO IT YOURSELF)
		void setFPSLimit(double num) {
			num = std::clamp(num, 1.0, 999.0);
			pendingFrameTime = 1.0 / num;
		}
		// updates per second, 1-999
		// Use applyNewLimits to apply changes. (IS DONE IN THE ENGINE LOOP. DO NOT DO IT YOURSELF)
		void setUPSLimit(double num) {
			num = std::clamp(num, 1.0, 999.0);
			pendingUpdateTime = 1.0/num;
		}
		double getFPSLimit() const { return 1.0 / pendingFrameTime; }
		double getUPSLimit() const { return 1.0 / pendingUpdateTime; }
		
		void applyNewLimits() {
			updateTime = pendingUpdateTime;
			frameTime = pendingFrameTime;
			gameSpeed = pendingGameSpeed;
		}

		double getRunTime() const { return runTime; }
		double getSleepTime() const { return sleepTime; }
		uint64_t getTotalUpdates() { return totalUpdates; }
		uint64_t getTotalUpdates() const { return totalFrames; }

		double getGameSpeed() { return pendingGameSpeed; }
		// minimum is 0.01
		void setGameSpeed(double speed) { pendingGameSpeed = speed<0.01?0.01:speed; }

		// real fps
		double getFPS() const { return real_fps; }
		// real ups
		double getUPS() const { return real_ups; }

		void print(PrintFlags flags) {
			//double timeDiff = GetSystemTime() - startTime;
			// time since start
			//if (printTss) {
			//	std::string formatBetween = FormatTime(timeDiff, false, FormatTimeMS|FormatTimeS | FormatTimeM | FormatTimeH);
			//	log::out << "TSS: " << formatBetween << "  ";
			//}
			if (flags & PrintEngineTime) {
				std::string formatRunTime = FormatTime(runTime, true, FormatTimeMS | FormatTimeS | FormatTimeM | FormatTimeH);
				std::string formatSleepTime = FormatTime(sleepTime, true, FormatTimeMS | FormatTimeS | FormatTimeM | FormatTimeH);
				log::out << "Run time: " << formatRunTime <<" Sleep time: "<<formatSleepTime<< "\n";
			}
			if(flags&PrintFramesUpdates)
				log::out << "Frames/Updates: " << totalFrames << " / " << totalUpdates << "\n";
			if(flags&PrintSamples)
				log::out << "FPS: " << (real_fps) << "  UPS: " << (real_ups) << "\n";
			
			// average
			//log::out << "FPS: " << (totalFrames/timeDiff) << "  UPS: " << (totalUpdates/timeDiff) << "\n";
		}

		uint64_t totalLoops = 0; // FEATURE: maybe nice?
	private:
		double startTime = 0;
		double runTime = 0;
		double sleepTime = 0; // time sleeping, when skipping frame

		uint64_t totalFrames = 0;
		uint64_t totalUpdates = 0;

		double update_accumulator=0;
		double frame_accumulator=0;

		double real_fps = 0;
		double real_ups = 0;

		double gameSpeed = 1.;

		double pendingUpdateTime;
		double pendingFrameTime;
		double pendingGameSpeed=1.;

		// one sample per round in while-loop
		static const int sample_count = 20;
		double sample_fps[sample_count]{};
		double sample_ups[sample_count]{};
		double sample_delta[sample_count]{};
		int sample_index = 0;
		void incrUpdates() {
			sample_ups[sample_index]++;
			totalUpdates++;
		}
		void incrFrames() {
			sample_fps[sample_index]++;
			totalFrames++;
		}
		// will calculate real fps/ups from samples
		void nextSample(double delta) {
			sample_delta[sample_index] = delta;
			real_fps = 0;
			real_ups = 0;
			double sumDelta = 0;
			for (int i = 0; i < sample_count; i++) {
				real_fps += sample_fps[i];
				real_ups += sample_ups[i];
				sumDelta += sample_delta[i];
			}
			real_fps /= sumDelta;
			real_ups /= sumDelta;

			sample_index++;
			if (sample_index >= sample_count) {
				sample_index = 0;
			}
			sample_fps[sample_index]=0;
			sample_ups[sample_index]=0;
			sample_delta[sample_index]=0;
		}

		double updateTime = 0;
		double frameTime = 0;

		friend class Engone;
	};
}
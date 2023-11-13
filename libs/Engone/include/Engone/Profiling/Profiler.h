#pragma once

#include "Engone/Core/ExecutionControl.h"

#include "Engone/Rendering/UIRenderer.h"

namespace engone {
	class Profiler : public Execution {
	public:
		Profiler();

		void reset(); // next round for time sections?

		void updateSection(const std::string& name, double seconds);

		void render(engone::LoopInfo& info);

		class Sampler {
		public:
			void increment();
			void next(double delta);
			double getAverage();

		private:
			static const uint32 MAX_SAMPLES = 20;
			uint32 totalSamples = 0;
			uint32 sampleIndex = 0;
			uint32 countSamples[MAX_SAMPLES];
			double deltaSamples[MAX_SAMPLES];

			double average = 0;
			double avgDelay = 0;
		};
		Sampler& getSampler(void* key);

		//void show();
		//void hide();

		struct ProfilingData {
			bool show=false;
		};
		struct TimedSection {
			// public
			std::string name;
			double loopSeconds = 0; // can be displayed
			int loopCount = 0;
			double totalSeconds = 0;

			// private
			double accSeconds = 0; // accumulation
			int accCount = 0; // accumulation
		};
		std::vector<TimedSection>& getSections();

		//class Graph {
		//public:
		//	Graph() = default;
		//	Graph(ui::Color color, float offsetY) : color(color), offsetY(offsetY){};
		//	~Graph();

		//	void start();

		//	void render(LoopInfo& info);
		//	static void RenderBack(LoopInfo& info);
		//	void plot(double time);

		//private:

		//	ui::Color color;
		//	double originTime = 0;
		//	float offsetX = 0;
		//	float offsetY = 0;
		//	float zoom = 1;

		//	static const int MAX_POINTS=60*100;
		//	int pushIndex=0;
		//	float* points = nullptr;
		//};
		
		struct Graph {
			Graph() = default;
			~Graph();
			static const int MAX_POINTS = 60 * 100;
			ui::Color color;
			int pushIndex = 0;
			float* points = nullptr;
			float offsetY = 0;

			void plot(double time);
		};

		Graph& getGraph(const std::string& name);

		//void startGraphs() {
		//	updateGraph.start();
		//	renderGraph.start();
		//}
		//Graph updateGraph{ {0,0,1,1},-15 };
		//Graph renderGraph{ {1,0,0,1},-5 };

	private:
		std::unordered_map<std::string, uint32> m_timedSectionsMap;
		
		void sort();

		float offsetX = 0;
		float zoom = 1;

		void renderBasicDebugInfo(LoopInfo& info);
		void renderGraphs(LoopInfo& info);
		void renderGraph(LoopInfo& info, Graph& graph);

		std::unordered_map<void*, Sampler> m_samples;
		std::unordered_map<void*, ProfilingData> m_profilingData;

		std::unordered_map<std::string, Graph> m_graphs;

		std::vector<TimedSection> m_timedSections;
	};
}
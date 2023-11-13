#pragma once

#include "Engone/LoopInfo.h"
// #include "Engone/Utilities/Locks.h"
// #include "Engone/Utilities/Thread.h"
#include "Engone/Utilities/TimeUtility.h"
#include "Engone/PlatformModule/PlatformLayer.h"

namespace engone {

	class Execution {
	public:
		Execution(std::string name, int priority=0);

		virtual void update(LoopInfo& info) {}
		virtual void render(LoopInfo& info) {}
		
		void setExecutionPriority(int priority);
		int getExecutionPriority();
		const std::string getExecutionName();

	private:
		std::string m_executionName;
		int m_priority = 0;
		void* funcPtr;

		friend class ExecutionControl;
	};

	class ExecutionTimer {
	public:
		double delta = 0;
		double accumulator = 0;
		double runtime = 0;
		double startTime = 0;

		double aimedDelta = 1/60.f;

		void step();
		
		// rename function? enoughAccumulation, hasAccumulation?, checkAccumulator?
		bool accumulate();
	
	private:
		double now = 0;
		double lastTime = 0;
	};
	class ExecutionControl {
	public:
		static const int UPDATE=0;
		static const int RENDER=1;

		ExecutionControl() = default;
		~ExecutionControl();
		void cleanup();

		void addExecution(Execution* execution);
		Execution* getExecution(const std::string& name);
		void removeExecution(const std::string& name);
		void removeExecution(Execution* execution);

		// use ExecutionControl::UPDATE/RENDER as type
		void execute(LoopInfo& info, int type);
	
	private:

		uint32_t m_executionIndex=0;
		Mutex m_mutex;
		std::vector<Execution*> m_executions;

		std::unordered_map<std::string, Execution*> m_quickAccess;

		// Sort executions based on priority. Highest first.
		void sort();
	};
}
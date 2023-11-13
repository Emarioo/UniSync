#pragma once

// #include "Engone/LoopInfo.h"
// #include "Engone/Logger.h"

// namespace engone {

// 	class RenderComponent {
// 	public:
// 		RenderComponent() = default;
// 		RenderComponent(const std::string& name, int priority = 0);

// 		virtual void render(LoopInfo& info) = 0;

// 		// add getter & setter
// 		void setName(const std::string& name);
// 		void setPriority(int priority);
// 		const std::string& getName();
// 		int getPriority();
// 	private:
// 		std::string m_name = "Undefined";
// 		int m_priority = 0;

// 		friend class RenderPipeline;
// 	};
// 	class RenderPipeline {
// 	public:
// 		RenderPipeline() = default;
// 		~RenderPipeline();

// 		// direct as true will apply component directly.
// 		// direct will add component to a queue and apply changes later.
// 		void addComponent(RenderComponent* component, bool direct=false);
// 		// You need to free the returned pointer in some way.
// 		// If is is nullptr then the engine already did it.
// 		RenderComponent* removeAction(const std::string& name);
// 		// returns first action with said name
// 		RenderComponent* getComponent(const std::string& name);
// 		// returns actions with said name
// 		std::vector<RenderComponent*> getComponents(const std::string& name);
// 		const std::vector<RenderComponent*>& getList();
// 		void render(LoopInfo& info);
		
// 		// called by the render function, also called by engone before the game loop
// 		void applyChanges();

// 		void setActivePipeline();

// 	private:
// 		std::mutex m_queueMutex;
// 		std::mutex m_componentMutex;
// 		std::vector<RenderComponent*> m_queue;
// 		std::vector<RenderComponent*> m_components; // also known as THE LIST
// 		std::unordered_map<std::string, RenderComponent*> m_quickAccess;

// 		void forceAddComponent(RenderComponent* component);
// 	};
// 	RenderPipeline* GetActivePipeline();
// }
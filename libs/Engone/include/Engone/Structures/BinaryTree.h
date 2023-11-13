namespace engone{
    // Value template requires <, >, == operators
	//template<typename Value>
	// BinaryTree is not finished, just copied from tracker. needs testing
	class BinaryTree;
	class BinaryNode {
	public:
		typedef int Value;

		BinaryNode() = default;

		// returns true if added, false if ptr already exists, or if something failed
		bool add(BinaryTree* tree, Value ptr);

		bool find(BinaryTree* tree, Value value);

		// the returned node was detached when you broke a node. The other node replaced the removed node.
		// returned node is nullptr if no node was detached
		bool remove(BinaryTree* tree, uint32_t& ref, Value ptr);
		void replace(BinaryTree* tree, Value ptr, uint32_t index);
		void reattach(BinaryTree* tree, uint32_t index);

		Value m_value;
		uint32_t left;
		uint32_t right;
	};
	class BinaryTree {
	public:
		typedef int Value;
		BinaryTree() = default;
		~BinaryTree() { cleanup(); }
		void cleanup();

		// returns true if added, false if ptr already exists
		bool add(Value value);
		// returns true if ptr was removed, false if it didn't exist
		bool remove(Value value);
		bool find(Value value);
		//void print() const;

		// free memory

		//void printNodes() const;

	private:
		uint32_t rootIndex = 0;
		uint32_t capacity = 0; // in count
		uint32_t head = 0; // int count
		// only change the allocation with resize
		BinaryNode* data = nullptr;

		BinaryNode* getNode(uint32_t index) const;
		uint32_t newNode();
		BinaryNode* getRoot() const;

		// size is the count of nodes.
		bool resize(uint32_t size);

		friend class BinaryNode;

	};

	//void CountingTest(int times, int numElements, std::function<int()> func);
}
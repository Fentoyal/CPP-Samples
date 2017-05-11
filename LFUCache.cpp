/*************************************
This is a sample Least Frequently Used Cache that supports all operation in O(1)
Very simple but it does work!
*****************************/
class LFUCache {
	struct FrequencyNode;
	struct RecencyNode { int key, value; list<FrequencyNode>::iterator iter; };
	struct FrequencyNode{ int frequency; list<RecencyNode> rec_list; };
	list<FrequencyNode> freq_list;
	unordered_map<int, list<RecencyNode>::iterator> hash;
	int m_capacity;
	list<RecencyNode>::iterator touch(list<RecencyNode>::iterator node_iter) {
		auto cur_iter = node_iter->iter;
		auto next_iter = next(cur_iter);
		if (next_iter == freq_list.end() || next_iter->frequency != cur_iter->frequency + 1)
			next_iter = freq_list.insert(next_iter, FrequencyNode{ cur_iter->frequency + 1, list<RecencyNode>() });
		node_iter->iter = next_iter;
		next_iter->rec_list.splice(next_iter->rec_list.begin(), cur_iter->rec_list, node_iter);
		if (cur_iter->rec_list.empty())
			freq_list.erase(cur_iter);
		return node_iter;
	}
	void evict() {
		if (hash.size() < m_capacity)
			return;
		auto to_remove = freq_list.front().rec_list.back();
		hash.erase(to_remove.key);
		freq_list.front().rec_list.pop_back();
		if (freq_list.front().rec_list.empty())
			freq_list.pop_front();
	}
public:
	LFUCache(int capacity) : m_capacity(capacity) {}
	int get(int key) {
		auto find_iter = hash.find(key);
		if (find_iter != hash.end())
			return touch(find_iter->second)->value;
		return -1;
	}
	void put(int key, int value) {
		if (m_capacity <= 0) return;
		auto find_iter = hash.find(key);
		if (find_iter != hash.end())
		{
			touch(find_iter->second)->value = value;
			return;
		}
		evict();
		freq_list.emplace_front();
		freq_list.front().rec_list.push_front({ key, value, freq_list.begin() });
		hash[key] = touch(freq_list.front().rec_list.begin());
	}
};

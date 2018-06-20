#ifndef M_REDIS_CONNECTION_INCLUDE
#define M_REDIS_CONNECTION_INCLUDE

#include "redis_wrapper_config.hpp"

//http://www.cnblogs.com/hjwublog/p/5639990.html
//https://redis.io/commands/setbit


// redis����
class RedisConnection {
public:
	shard_ptr_t<_rediscontext_> _context;

public:
	RedisConnection();

	RedisConnection(shard_ptr_t<_rediscontext_> context);

	RedisConnection(const RedisConnection& conn);

	~RedisConnection();

	RedisConnection& operator=(const RedisConnection& conn);

	bool connected()const {
		return (_context != 0);
	}

	unsigned long long connectionid()const;

	////////////////////////////////////////////////////////////////////

	// ��ʱ����
	bool expire(const char* key, time_t expire);

	// 1Ϊkey���ڣ�0��ʾkey������
	int del(const char* key);

	template<typename T>
	int dels(const T& keys);

	////////////////////////////////////////////////////////////////////

	template<typename T>
	void set(const char* key, T value);

	void set(const char* key, const std::string& value);

	void set(const char* key, const char* value, unsigned int len);
	
	// ����key��Ӧ��ֵΪString���͵�value�����key�Ѿ������򷵻�false
	template<typename T>
	bool setnx(const char* key, T value);
	
	bool setnx(const char* key, const std::string& value);
	
	bool setnx(const char* key, const char* value, unsigned int len);

	// ����ֵ��������һ����ʱ 
	template<typename T>
	void setex(const char* key, T value, time_t expire);
	
	void setex(const char* key, const std::string& value, time_t expire);
	
	void setex(const char* key, const char* value, unsigned int len, time_t expire);
	
	////////////////////////////////////////////////////////////////////

	template<typename T>
	void get(const char* key, T& value);

	void get(const char* key, std::string& value);
	
	void get(const char* key, char* value, unsigned int len);

	////////////////////////////////////////////////////////////////////

	void incrby(const char* key, int step);

	template<typename T>
	void incrby(const char* key, int step, T& new_value);

	template<typename T>
	void decrby(const char* key, int step, T& new_value);
	
	void decrby(const char* key, int step);

	////////////////////////////////////////////////////////////////////

	// ���س���
	int strlen(const char* key);

	// �����³���
	int append(const char* key, const std::string& app_value);
	
	int append(const char* key, const char* value, unsigned int len);

	////////////////////////////////////////////////////////////////////

	// �����޸ĺ��ַ�������
	int setrange(const char* key, int beg_idx, const char* value, unsigned int len);
	
	int setrange(const char* key, int beg_idx, const std::string& value);
	
	void getrange(const char* key, int beg_idx, int end_idx, std::string& value);

	////////////////////////////////////////////////////////////////////

	// ����ԭλ��ֵ��ֻ����0��1
	int setbit(const char* key, unsigned int offset, int value);
	
	int getbit(const char* key, unsigned int offset);

	////////////////////////////////////////////////////////////////////

	// ����push������
	template<typename T>
	int lpush(const char* key, const T& value);

	int lpush(const char* key, const std::string& value);
	
	template<typename T>
	int lpushs(const char* key, const T& values);

	////////////////////////////////////////////////////////////////////

	template<typename T>
	void lrange(const char* key, int beg_idx, int end_idx, T& values, typename T::value_type*);
	
	template<typename T>
	void lrange(const char* key, int beg_idx, int end_idx, T& values, std::string*);

	////////////////////////////////////////////////////////////////////

	// ��Ԫ�ز�����ʱ����False
	template<typename T>
	bool lpop(const char* key, T& value);
	
	bool lpop(const char*key, std::string& value);
	
	bool lpop(const char*key, char* value, unsigned int len);

	////////////////////////////////////////////////////////////////////

	template<typename T>
	int rpush(const char* key, const T& value);
	
	int rpush(const char* key, const std::string& value);
	
	template<typename T>
	int rpushs(const char* key, const T& values);
	
	////////////////////////////////////////////////////////////////////

	// ��Ԫ�ز�����ʱ����False
	template<typename T>
	bool rpop(const char* key, T& value);
	
	bool rpop(const char*key, std::string& value);
	
	bool rpop(const char*key, char* value, unsigned int len);

	////////////////////////////////////////////////////////////////////

	int llen(const char* key);

	// ���Ԫ�ز����ڷ���false
	template<typename T>
	bool lindex(const char* key, int idx, T&value);
	
	bool lindex(const char* key, int idx, std::string&value);
	
	bool lindex(const char* key, int idx, char* value, unsigned int len);

	////////////////////////////////////////////////////////////////////

	/*
	   ���б� key �±�Ϊ index ��Ԫ�ص�ֵ����Ϊ value.
	   �� index ����������Χ�����һ�����б�( key ������)���� LSET ʱ������һ������.
	 */
	template<typename T>
	void lset(const char* key, int idx, T value);
	
	void lset(const char* key, int idx, const std::string& value);
	
	void lset(const char* key, int idx, const char* value, unsigned int len);
	
	template<int N>
	void lset(const char* key, int idx, const char(&value)[N]);

	////////////////////////////////////////////////////////////////////

	/*
		���ݲ��� count ��ֵ���Ƴ��б�������� value ��ȵ�Ԫ��.
		count ��ֵ���������¼��֣�
		count > 0 : �ӱ�ͷ��ʼ���β�������Ƴ��� value ��ȵ�Ԫ�أ�����Ϊ count ��
		count < 0 : �ӱ�β��ʼ���ͷ�������Ƴ��� value ��ȵ�Ԫ�أ�����Ϊ count �ľ���ֵ��
		count = 0 : �Ƴ����������� value ��ȵ�ֵ��
	*/
	template<typename T>
	int lrem(const char* key, int idx, T value);

	/*
		��һ���б�����޼�(trim)������˵�����б�ֻ����ָ�������ڵ�Ԫ�أ�����ָ������֮�ڵ�Ԫ�ض�����ɾ����
	*/
	void ltrim(const char*key, int beg_idx, int end_idx);

	// �����б�ĳ���
	template<typename T1, typename T2>
	int linsert(const char* key, bool b_or_a, const T1& value1, const T2& value2);
	
	int linsert(const char* key, bool b_or_a, const std::string& value1, const std::string&value2);

	/*
		һ��ԭ��ʱ���ڣ�ִ��������������:
		���б� source �е����һ��Ԫ��(βԪ��)�����������ظ��ͻ��ˡ�
		�� source ������Ԫ�ز��뵽�б� destination ����Ϊ destination �б�ĵ�ͷԪ�ء�
	*/
	template<typename T>
	bool rpoplpush(const char*key1, const char*key2, T& value);
	
	bool rpoplpush(const char*key1, const char*key2, std::string& value);

	////////////////////////////////////////////////////////////////////

	// ����1��ʾ���裬����0��ʾ����
	template<typename T>
	int hset(const char* key, const char* field, const T& value);
	
	int hset(const char* key, const char* field, const std::string& value);
	
	int hset(const char* key, const char* field, const char* value, unsigned int len);
	
	template<typename T>
	void hget(const char* key, const char* field, T& value);

	void hget(const char* key, const char* field, std::string& value);
	
	void hget(const char* key, const char* field, char* value, unsigned int len);

	template<typename T1, typename T2>
	void hgetall(const char* key, std::map<T1, T2>& values);
	
	template<typename T1, typename T2, typename T3>
	void hgetall(const char* key, T1& values, std::pair<T2, T3>*);

	////////////////////////////////////////////////////////////////////

	// ���key�Ѵ��ڣ�������ʧ��
	template<typename T>
	bool hsetnx(const char* key, const char* field, const T& value);
	
	bool hsetnx(const char* key, const char* field, const std::string& value);
	
	bool hsetnx(const char* key, const char* field, const char* value, unsigned int len);
	
	template<int N>
	bool hsetnx(const char* key, const char* field, const char(&value)[N]);

	////////////////////////////////////////////////////////////////////

	template<typename T1, typename T2>
	void hmset(const char* key, const std::map<T1, T2>& values);

	template<typename T1, typename T2, typename T3>
	void hmset(const char* key, const T1& values, std::pair<T2, T3>*);

	// values��in-outֵ
	template<typename T1, typename T2>
	void hmget(const char* key, std::vector<TriangleValule<T1, T2> >&values);

	////////////////////////////////////////////////////////////////////

	template<typename T>
	void hincrby(const char* key, const char* field, int step, T& new_value);
	
	void hincrby(const char* key, const char* field, int step);

	bool hexists(const char* key, const char*field);
	
	int  hlen(const char* key);
	
	bool hdel(const char* key, const char*field);
	
	template<typename T>
	bool hdel(const char*key, const T& field);

	// TҪ����һ������
	template<typename T>
	void hkeys(const char*key, T& values);

	// TҪ����һ������
	template<typename T>
	void hvals(const char*key, T& values);

	////////////////////////////////////////////////////////////////////

	// ���سɹ�����ĸ���
	template<typename T>
	int sadd(const char*key, const T& value);
	
	int sadd(const char*key, const std::string&value);
	
	template<typename T>
	int sadds(const char*key, const T& values);

	////////////////////////////////////////////////////////////////////

	// TҪ����һ������,���ؼ��� key �е����г�Ա
	template<typename T>
	void smembers(const char*key, T&values);
	
	template<typename T>
	bool spop(const char*key, T&value);
	
	bool spop(const char*key, std::string&value);
	
	bool spop(const char*key, char*value, unsigned int len);

	////////////////////////////////////////////////////////////////////

	template<typename T>
	bool srem(const char*key, const T& field);

	bool srem(const char*key, const char* field);

	////////////////////////////////////////////////////////////////////

	int scard(const char*key);

	bool sismember(const char*key, const char* field);
	
	template<typename T>
	bool sismember(const char*key, const T& field);
	
	template<typename T>
	bool smove(const char* src_key, const char* dst_key, const T& field);
	
	bool smove(const char* src_key, const char* dst_key, const char* field);

	////////////////////////////////////////////////////////////////////

	// �������һ��Ԫ��
	template<typename T>
	bool srandmember(const char*key, T&value);
	
	bool srandmember(const char*key, std::string&value);
	
	// TҪ����һ���������������count��Ԫ��
	template<typename T>
	void srandmember(const char*key, T&values, int count);

	////////////////////////////////////////////////////////////////////

	template<typename T>
	void sdiff(const char* key, const std::vector<std::string>& other_keys, T& values);
	
	template<typename T>
	void sunion(const char* key, const std::vector<std::string>& other_keys, T& values);

	template<typename T>
	void sinter(const char* key, const std::vector<std::string>& other_keys, T& values);
	
	/*
		�����������ú� SDIFF ���ƣ�������������浽 destination ���ϣ������Ǽ򵥵ط��ؽ������
		��� destination �����Ѿ����ڣ����串�ǡ�
		destination ������ key ����
	*/
	int sdiffstore(const char* key, const std::vector<std::string>& other_keys);
	
	int sunionstore(const char* key, const std::vector<std::string>& other_keys);
	
	int sinterstore(const char* key, const std::vector<std::string>& other_keys);

	////////////////////////////////////////////////////////////////////

	template<typename T>
	int zadd(const char*key, float score, const T& member);

	int zadd(const char*key, float score, const std::string& member);

	// TҪ������������Ԫ����pair�ṹ
	template<typename T>
	int zadds(const char*key, const T& values);

	////////////////////////////////////////////////////////////////////

	template<typename T>
	void zrange(const char*key, int beg_idx, int end_idx, T& values);
	
	template<typename T1, typename T2>
	void zrangewithscores(const char* key, int beg_idx, int end_idx,
		std::map<T1, T2>& values);

	////////////////////////////////////////////////////////////////////

	template<typename T>
	bool zrem(const char*key, const T& member);
	
	bool zrem(const char*key, const std::string& member);
	
	template<typename T>
	int  zrem(const char*key, const T& members);

	////////////////////////////////////////////////////////////////////

	template<typename T>
	void zrevrange(const char*key, int beg_idx, int end_idx, T& values);
	
	template<typename T1, typename T2>
	void zrevrangewithscores(const char* key, int beg_idx, int end_idx, std::map<T1, T2>& values);

	////////////////////////////////////////////////////////////////////

	bool zrank(const char* key, const char* member, int& rank);
	
	bool zrevrank(const char* key, const char* member, int& rank);

	////////////////////////////////////////////////////////////////////

	void zcard(const char* key, int& count);
	
	// min �� max֮��ĸ���
	void zcount(const char* key, double min, double max, int& count);

	void zincrby(const char* key, const char* member, double incr, double& score);

	////////////////////////////////////////////////////////////////////

	// �������� key �У����� score ֵ���� min �� max ֮��(�������� min �� max )�ĳ�Ա�����򼯳�Ա�� score ֵ����(��С����)��������
	template<typename T1, typename T2, typename T3>
	void zrangebyscore(const char* key, T1 min, T2 max, T3& values);
	
	template<typename T1, typename T2, typename T3>
	void zrevrangebyscore(const char* key, T1 min, T2 max, T3& values);
	
	////////////////////////////////////////////////////////////////////

	void zscore(const char* key, const char* member, double& score);
};



#endif
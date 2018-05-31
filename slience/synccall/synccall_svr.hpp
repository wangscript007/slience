#ifndef M_SYNCCALL_SVR_INCLUDE
#define M_SYNCCALL_SVR_INCLUDE

#include "slience/synccall/config.hpp"
#include "slience/synccall/server_handler.hpp"
#include "slience/synccall/synccall_coclient.hpp"
#include "slience/base/thread.hpp"
#include <stdlib.h>
M_SYNCCALL_NAMESPACE_BEGIN

#ifdef M_OPEN_DEBUG_LOG
#define M_PRINT_DEBUG_LOG(info) printf(info)
#else  
#define M_PRINT_DEBUG_LOG(info)
#endif

class ScClient;
class ScServer;

struct IScServer {
	friend class ScIo;
protected:
	virtual void OnConnected(netiolib::TcpSocketPtr& clisock) = 0;
	virtual void OnConnected(netiolib::TcpConnectorPtr& clisock, SocketLib::SocketError error) = 0;
	virtual void OnDisconnected(netiolib::TcpSocketPtr& clisock) = 0;
	virtual void OnDisconnected(netiolib::TcpConnectorPtr& clisock) = 0;
	virtual void OnReceiveData(netiolib::TcpSocketPtr& clisock, netiolib::Buffer& buffer) = 0;
	virtual void OnReceiveData(netiolib::TcpConnectorPtr& clisock, netiolib::Buffer& buffer) = 0;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class ScIo : public netiolib::NetIo {
public:
	ScIo(IScServer* server) {
		_server = server;
	}
protected:
	virtual void OnConnected(netiolib::TcpSocketPtr& clisock) {
		_server->OnConnected(clisock);
	}
	virtual void OnConnected(netiolib::TcpConnectorPtr& clisock, 
		SocketLib::SocketError error) {
		_server->OnConnected(clisock, error);
	}
	virtual void OnDisconnected(netiolib::TcpSocketPtr& clisock) {
		_server->OnDisconnected(clisock);
	}
	virtual void OnDisconnected(netiolib::TcpConnectorPtr& clisock) {
		_server->OnDisconnected(clisock);
	}
	virtual void OnReceiveData(netiolib::TcpSocketPtr& clisock,
		netiolib::Buffer& buffer) {
		_server->OnReceiveData(clisock, buffer);
	}
	virtual void OnReceiveData(netiolib::TcpConnectorPtr& clisock,
		netiolib::Buffer& buffer) {
		_server->OnReceiveData(clisock, buffer);
	}
protected:
	IScServer* _server;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class ScServer : public IScServer {
	friend class ScIo;
public:
	ScServer();
	~ScServer();
	bool Register(const std::string& ip, unsigned short port, IServerHandler* handler);
	CoScClient* CreateCoScClient();
	void Start(unsigned int thread_cnt, bool isco = false);
	void Stop();
protected:
	void OnConnected(netiolib::TcpSocketPtr& clisock);
	void OnConnected(netiolib::TcpConnectorPtr& clisock, SocketLib::SocketError error);
	void OnDisconnected(netiolib::TcpSocketPtr& clisock);
	void OnDisconnected(netiolib::TcpConnectorPtr& clisock);
	void OnReceiveData(netiolib::TcpSocketPtr& clisock, netiolib::Buffer& buffer);
	void OnReceiveData(netiolib::TcpConnectorPtr& clisock, netiolib::Buffer& buffer);
	base::s_uint16_t UniqueId(const std::string& ip, unsigned short port);

private:
	ScIo _io;
	netiolib::HeartBeatMng _heartmng;
	std::list<base::thread*> _threads;
	IServerHandler* _handlers[0xFFFF];
};

inline ScServer::ScServer() 
	:_io(this) {
	memset(_handlers, 0, sizeof(_handlers));

	netiolib::Buffer buffer;
	ScKeepAlive msg;
	msg.Write(buffer);
	_heartmng.SetConnSndBuffer(buffer);
}

inline ScServer::~ScServer() {
	for (std::list<base::thread*>::iterator iter = _threads.begin();
		iter != _threads.end(); ++iter) {
		(*iter)->join();
		delete (*iter);
	}
	_threads.clear();

	for (base::s_uint16_t idx = 0; idx < (sizeof(_handlers) / sizeof(IServerHandler*)); ++idx) {
		delete _handlers[idx];
	}
}

inline bool ScServer::Register(const std::string& ip, unsigned short port, IServerHandler* handler) {
	if (_io.ListenOne(ip, port)) {
		base::s_uint16_t id = UniqueId(ip, port);
		_handlers[id] = handler;
		return true;
	}
	else {
		return false;
	}
}

inline CoScClient* ScServer::CreateCoScClient() {
	CoScClient* client = new CoScClient;
	client->_io = &_io;
	client->_heartmng = &_heartmng;
	return client;
}

inline void ScServer::Start(unsigned int thread_cnt, bool isco) {
	_io.Start(thread_cnt, isco);
	_heartmng.Start(thread_cnt);
}

inline void ScServer::Stop() {
	_io.Stop();
	_heartmng.Stop();
}

inline void ScServer::OnConnected(netiolib::TcpSocketPtr& clisock) {
	M_PRINT_DEBUG_LOG("onconnected......\n");
	std::string ip = clisock->LocalEndpoint().Address();
	base::s_uint16_t port = clisock->LocalEndpoint().Port();
	_ScInfo_* pscinfo = new _ScInfo_;
	pscinfo->id = UniqueId(ip, port);
	clisock->GetSocket().SetData(pscinfo);
}

inline void ScServer::OnConnected(netiolib::TcpConnectorPtr& clisock, SocketLib::SocketError error) {
}

inline void ScServer::OnDisconnected(netiolib::TcpSocketPtr& clisock) {
	M_PRINT_DEBUG_LOG("ondisconnected......\n");
	_ScInfo_* pscinfo = (_ScInfo_*)clisock->GetSocket().GetData();
	delete pscinfo;
}

inline void ScServer::OnDisconnected(netiolib::TcpConnectorPtr& clisock) {
	_CoScInfo_* pscinfo = (_CoScInfo_*)clisock->GetExtData();
	pscinfo->mutex.lock();
	int co_id = pscinfo->co_id;
	int thr_id = pscinfo->thr_id;
	pscinfo->co_id = -1;
	pscinfo->thr_id = 0;
	pscinfo->valid = false;
	pscinfo->buffer.Clear();
	pscinfo->mutex.unlock();
	if (co_id != -1)
		coroutine::CoroutineTask::addResume(thr_id, co_id);
}

inline void ScServer::OnReceiveData(netiolib::TcpSocketPtr& clisock, netiolib::Buffer& buffer) {
	M_PRINT_DEBUG_LOG("onreceivedata......\n");
	_ScInfo_* pscinfo = (_ScInfo_*)clisock->GetSocket().GetData();
	if (!pscinfo) {
		printf("pscinfo is null\n");
		return;
	}
	base::s_uint32_t msgid = 0;
	buffer.Read(msgid);
	if (msgid == M_KEEPALIVE_ID) {
		ScKeepAliveAck msg;
		buffer.Clear();
		msg.Write(buffer);
		clisock->Send(buffer.Data(), buffer.Length());
	}else if (_handlers[pscinfo->id]) {
		base::s_int32_t len = sizeof(base::s_uint32_t);
		buffer.CutData(len*-1);
		base::s_uint32_t way_type = 0;
		buffer.Read(way_type);
		base::s_uint32_t msg_type = 0;
		buffer.Read(msg_type);
		base::s_uint32_t pack_idx = 0;
		buffer.Read(pack_idx);
		pscinfo->buffer.Clear();
		pscinfo->buffer.Write(way_type);
		pscinfo->buffer.Write(msg_type);
		pscinfo->buffer.Write(pack_idx);
		// don't ask why the way_type is 666 or 999
		if (way_type == M_ONEWAY_TYPE) {
			_handlers[pscinfo->id]->OnOneWayDealer(msg_type, buffer);
			clisock->Send(pscinfo->buffer.Data(), pscinfo->buffer.Length());
		}
		else if (way_type == M_TWOWAY_TYPE) {
			_handlers[pscinfo->id]->OnTwoWayDealer(msg_type, buffer, pscinfo->buffer);
			clisock->Send(pscinfo->buffer.Data(), pscinfo->buffer.Length());
		}
		else {
			printf("error way_type(%d)\n", way_type);
		}
	}
}

inline void ScServer::OnReceiveData(netiolib::TcpConnectorPtr& clisock, netiolib::Buffer& buffer) {
	_heartmng.OnReceiveData(clisock);
	base::s_uint32_t msgid = 0;
	buffer.Read(msgid);
	if (msgid != M_KEEPALIVE_ACK_ID) {
		base::s_int32_t len = sizeof(base::s_uint32_t);
		buffer.CutData(len*-1);
		_CoScInfo_* pscinfo = (_CoScInfo_*)clisock->GetExtData();
		int co_id = pscinfo->co_id;
		int thr_id = pscinfo->thr_id;
		pscinfo->co_id = -1;
		pscinfo->thr_id = 0;
		pscinfo->buffer.Clear();
		pscinfo->buffer.Swap(buffer);
		coroutine::CoroutineTask::addResume(thr_id, co_id);
	}
}

inline base::s_uint16_t ScServer::UniqueId(const std::string& ip, unsigned short port) {
	return port;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


M_SYNCCALL_NAMESPACE_END
#endif
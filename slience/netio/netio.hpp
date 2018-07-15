/*----------------------------------------------------------------
// Copyright (C) 2017 public
//
// �޸��ˣ�xiaoquanjie
// ʱ�䣺2017/11/10
//
// �޸��ˣ�xiaoquanjie
// ʱ�䣺
// �޸�˵����
//
// �汾��V1.0.0
//----------------------------------------------------------------*/

#ifndef M_NETIO_NETIO_INCLUDE
#define M_NETIO_NETIO_INCLUDE

#include "slience/netio/config.hpp"
M_NETIO_NAMESPACE_BEGIN

#ifdef M_PLATFORM_WIN
#pragma pack(1)
struct MessageHeader {
	int	timestamp;
	unsigned int size;
	void n2h();
	void h2n();
};
#pragma pack()
#else
struct __attribute__((__packed__)) MessageHeader {
	int	timestamp;
	unsigned int size;
	void n2h();
	void h2n();
};
#endif

class NetIo;
class TcpSocket;
class TcpConnector;
class HttpSocket;
class HttpConnector;
class SyncConnector;

typedef SocketLib::Buffer Buffer;
typedef shard_ptr_t<SocketLib::Buffer> BufferPtr;
typedef shard_ptr_t<TcpSocket>		   TcpSocketPtr;
typedef shard_ptr_t<TcpConnector>	   TcpConnectorPtr;
typedef shard_ptr_t<HttpSocket>		   HttpSocketPtr;
typedef shard_ptr_t<HttpConnector>	   HttpConnectorPtr;
typedef shard_ptr_t<SocketLib::TcpAcceptor<SocketLib::IoService> > TcpAcceptorPtr;
typedef shard_ptr_t<SyncConnector>  SyncConnectorPtr;

#ifndef lasterror
#define lasterror base::tlsdata<SocketLib::SocketError,0>::data()
#endif

class NetIo {
public:
	NetIo();
	NetIo(SocketLib::s_uint32_t backlog);

	virtual ~NetIo();

	// ����һ������
	bool ListenOne(const SocketLib::Tcp::EndPoint& ep);
	bool ListenOne(const std::string& addr, SocketLib::s_uint16_t port);

	// ����һ��http����
	bool ListenOneHttp(const SocketLib::Tcp::EndPoint& ep);
	bool ListenOneHttp(const std::string& addr, SocketLib::s_uint16_t port);

	// �첽����
	void ConnectOne(const SocketLib::Tcp::EndPoint& ep);
	void ConnectOne(const std::string& addr, SocketLib::s_uint16_t port);

	void ConnectOneHttp(const SocketLib::Tcp::EndPoint& ep);
	void ConnectOneHttp(const std::string& addr, SocketLib::s_uint16_t port);

	virtual void Start(unsigned int thread_cnt, bool isco = false);
	virtual void Stop();
	virtual void RunHandler();
	size_t  ServiceCount();

	// ��ȡ�����쳣
	SocketLib::SocketError GetLastError()const;
	SocketLib::IoService& GetIoService();
	SocketLib::s_uint32_t LocalEndian()const;

	/*
	*����������������Ϊ�麯�����Ա����ʵ��ҵ���ģʽ��������ģʽ����Ϣ���ַ�����
	*��֤ͬһ��socket���������������ĵ�����ѭOnConnected -> OnReceiveData -> OnDisconnected��˳��
	*��֤ͬһ��socket�����º����������ĵ��ö���ͬһ���߳���
	*/

	// ����֪ͨ,��������ﲻҪ����ҵ�񣬷�ֹ����
	virtual void OnConnected(TcpSocketPtr& clisock);
	virtual void OnConnected(TcpConnectorPtr& clisock, SocketLib::SocketError error);
	virtual void OnConnected(HttpSocketPtr& clisock);
	virtual void OnConnected(HttpConnectorPtr& clisock, SocketLib::SocketError error);

	// ����֪ͨ,��������ﲻҪ����ҵ�񣬷�ֹ����
	virtual void OnDisconnected(TcpSocketPtr& clisock);
	virtual void OnDisconnected(TcpConnectorPtr& clisock);
	virtual void OnDisconnected(HttpSocketPtr& clisock);
	virtual void OnDisconnected(HttpConnectorPtr& clisock);

	// ���ݰ�֪ͨ,��������ﲻҪ����ҵ�񣬷�ֹ����
	virtual void OnReceiveData(TcpSocketPtr& clisock, SocketLib::Buffer& buffer);
	virtual void OnReceiveData(TcpConnectorPtr& clisock, SocketLib::Buffer& buffer);
	virtual void OnReceiveData(HttpSocketPtr& clisock, HttpSvrRecvMsg& httpmsg);
	virtual void OnReceiveData(HttpConnectorPtr& clisock, HttpCliRecvMsg& httpmsg);

protected:
	void _Start(void*p);
	void _AcceptHandler(SocketLib::SocketError error, TcpSocketPtr& clisock, 
		TcpAcceptorPtr& acceptor);
	void _AcceptHttpHandler(SocketLib::SocketError error, HttpSocketPtr& clisock, 
		TcpAcceptorPtr& acceptor);

protected:
	NetIo(const NetIo&);
	NetIo& operator=(const NetIo&);

protected:
	SocketLib::IoService   _ioservice;
	SocketLib::s_uint32_t  _backlog;
	SocketLib::s_uint32_t  _endian;
	base::slist<base::thread*> _threadlist;
};

enum {
	E_STATE_START = 1,
	E_STATE_STOP,
	E_STATE_CLOSE,
};


template<typename T, typename SocketType>
class TcpBaseSocket : public enable_shared_from_this_t<T>
{
protected:
	struct _writerinfo_ {
		enum {
			E_MAX_BUFFER_SIZE = 1024 * 1024 * 3,
		};

		SocketLib::MutexLock lock;
		bool writing;
		SocketLib::Buffer msgbuffer1;
		SocketLib::Buffer msgbuffer2;

		_writerinfo_();
		~_writerinfo_();
	};

public:
	TcpBaseSocket(NetIo& netio);

	virtual ~TcpBaseSocket();

	const SocketLib::Tcp::EndPoint& LocalEndpoint()const;

	const SocketLib::Tcp::EndPoint& RemoteEndpoint()const;

	// ���������������ζ�����������Ͽ���������е�δ��������ݴ�����ͻ�Ͽ�
	void Close();

	bool Send(const SocketLib::Buffer* buffer);

	bool Send(const SocketLib::s_byte_t* data, SocketLib::s_uint32_t len);

	bool IsConnected()const;

	void SetExtData(void* data, void(*func)(void*data));

	void SetExtData(void* data);

	void* GetExtData();

	void SetKeepAlive(SocketLib::s_uint32_t timeo);

protected:
	void _WriteHandler(SocketLib::s_uint32_t tran_byte, SocketLib::SocketError error);

	inline void _CloseHandler();

	void _PostClose();

	void _Close();

	bool _TrySendData();

protected:
	NetIo& _netio;
	SocketType*  _socket;
	_writerinfo_ _writer;

	// endpoint
	SocketLib::Tcp::EndPoint _localep;
	SocketLib::Tcp::EndPoint _remoteep;

	// ״̬��־
	unsigned short _flag;
	void* _extdata;
	void(*_extdata_func)(void*data);
};

// for stream
template<typename T, typename SocketType>
class TcpStreamSocket : public TcpBaseSocket<T, SocketType>
{
protected:
	struct _readerinfo_ {
		SocketLib::s_byte_t*  readbuf;
		SocketLib::Buffer	  msgbuffer;
		MessageHeader		  curheader;

		_readerinfo_();
		~_readerinfo_();
	};

	_readerinfo_ _reader;

	void _ReadHandler(SocketLib::s_uint32_t tran_byte, SocketLib::SocketError error);

	// �ü������ݰ�������false��ζ�����ݰ��д�
	bool _CutMsgPack(SocketLib::s_byte_t* buf, SocketLib::s_uint32_t tran_byte);

	void _TryRecvData();

public:
	TcpStreamSocket(NetIo& netio);
};

// class tcpsocket
class TcpSocket :
	public TcpStreamSocket<TcpSocket, SocketLib::TcpSocket<SocketLib::IoService> >
{
	friend class NetIo;

public:
	TcpSocket(NetIo& netio)
		:TcpStreamSocket(netio) {
	}

	SocketLib::TcpSocket<SocketLib::IoService>& GetSocket() {
		return (*this->_socket);
	}

protected:
	void Init() {
		try {
			_remoteep = _socket->RemoteEndPoint();
			_localep = _socket->LocalEndPoint();
			_flag = E_STATE_START;
			shard_ptr_t<TcpSocket> ref = this->shared_from_this();
			_netio.OnConnected(ref);
			this->_TryRecvData();
		}
		catch (const SocketLib::SocketError& e) {
			lasterror = e;
		}
	}
};

// class tcpconnector
class TcpConnector 
	: public TcpStreamSocket<TcpConnector, SocketLib::TcpConnector<SocketLib::IoService> >{
public:
	TcpConnector(NetIo& netio);

	SocketLib::TcpConnector<SocketLib::IoService>& GetSocket();

	bool Connect(const SocketLib::Tcp::EndPoint& ep, SocketLib::s_uint32_t timeo_sec = -1);

	bool Connect(const std::string& addr, SocketLib::s_uint16_t port, SocketLib::s_uint32_t timeo_sec = -1);

	void AsyncConnect(const SocketLib::Tcp::EndPoint& ep, SocketLib::SocketError& error);

	void AsyncConnect(const std::string& addr, SocketLib::s_uint16_t port, SocketLib::SocketError& error);

protected:
	void _ConnectHandler(const SocketLib::SocketError& error, TcpConnectorPtr conector);
};

// for http
template<typename T, typename SocketType, typename HttpMsgType>
class HttpBaseSocket :
	public TcpBaseSocket<T, SocketType>
{
protected:
	struct _readerinfo_ {
		SocketLib::s_byte_t*  readbuf;
		HttpMsgType httpmsg;

		_readerinfo_();
		~_readerinfo_();
	};

	_readerinfo_ _reader;

	void _ReadHandler(SocketLib::s_uint32_t tran_byte, SocketLib::SocketError error);

	// �ü������ݰ�������false��ζ�����ݰ��д�
	bool _CutMsgPack(SocketLib::s_byte_t* buf, SocketLib::s_uint32_t tran_byte);

	void _TryRecvData();

public:
	HttpBaseSocket(NetIo& netio);
};

// class httpsocket
class HttpSocket :
	public HttpBaseSocket<HttpSocket, SocketLib::TcpSocket<SocketLib::IoService>, HttpSvrRecvMsg>
{
	friend class NetIo;
public:
	HttpSocket(NetIo& netio)
		:HttpBaseSocket(netio) {
	}

	SocketLib::TcpSocket<SocketLib::IoService>& GetSocket() {
		return (*this->_socket);
	}

	HttpSvrSendMsg& GetSvrMsg() {
		return _httpmsg;
	}

	void SendHttpMsg() {
		Send(_httpmsg._pbuffer);
		_httpmsg._pbuffer->Clear();
		_httpmsg._flag = 0;
	}

protected:
	void Init() {
		try {
			this->_remoteep = this->_socket->RemoteEndPoint();
			this->_localep = this->_socket->LocalEndPoint();
			this->_flag = E_STATE_START;
			shard_ptr_t<HttpSocket> ref = this->shared_from_this();
			this->_netio.OnConnected(ref);
			this->_TryRecvData();
		}
		catch (const SocketLib::SocketError& e) {
			lasterror = e;
		}
	}

	HttpSvrSendMsg _httpmsg;
};

class HttpConnector : public HttpBaseSocket<HttpConnector, SocketLib::TcpConnector<SocketLib::IoService>
	, HttpCliRecvMsg> {
public:
	HttpConnector(NetIo& netio);

	SocketLib::TcpConnector<SocketLib::IoService>& GetSocket();

	bool Connect(const SocketLib::Tcp::EndPoint& ep);

	bool Connect(const std::string& addr, SocketLib::s_uint16_t port);

	void AsyncConnect(const SocketLib::Tcp::EndPoint& ep);

	void AsyncConnect(const std::string& addr, SocketLib::s_uint16_t port);

protected:
	void _ConnectHandler(const SocketLib::SocketError& error, HttpConnectorPtr conector);
};

// ͬ��connector
class SyncConnector {
public:
	SyncConnector();

	~SyncConnector();

	bool Connect(const SocketLib::Tcp::EndPoint& ep, SocketLib::s_uint32_t timeo_sec = -1);

	bool Connect(const std::string& addr, SocketLib::s_uint16_t port, SocketLib::s_uint32_t timeo_sec = -1);

	const SocketLib::Tcp::EndPoint& LocalEndpoint()const;

	const SocketLib::Tcp::EndPoint& RemoteEndpoint()const;

	// ���������������ζ�����������Ͽ���������е�δ��������ݴ�����ͻ�Ͽ�
	void Close();

	bool Send(const SocketLib::s_byte_t* data, SocketLib::s_uint32_t len);

	bool IsConnected()const;

	SocketLib::Buffer* Recv();

	void SetTimeOut(SocketLib::s_uint32_t timeo);

protected:
	SocketLib::s_uint32_t _LocalEndian()const;

	SocketLib::Buffer* _CutMsgPack(SocketLib::s_byte_t* buf, SocketLib::s_uint32_t& tran_byte);

protected:
	SocketLib::IoService _ioservice;
	SocketLib::TcpConnector<SocketLib::IoService>* _socket;

	// endpoint
	SocketLib::Tcp::EndPoint _localep;
	SocketLib::Tcp::EndPoint _remoteep;

	MessageHeader		  _curheader;
	SocketLib::s_byte_t*  _readbuf;
	SocketLib::s_uint32_t _readsize;

	SocketLib::Buffer _sndbuffer;
	SocketLib::Buffer _rcvbuffer;
	// ״̬��־
	unsigned short _flag;
};

M_NETIO_NAMESPACE_END
#include "slience/netio/tsocket_impl.hpp"
#include "slience/netio/hsocket_impl.hpp"
#include "slience/netio/heartbeat.hpp"
#endif
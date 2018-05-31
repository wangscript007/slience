/*----------------------------------------------------------------
// Copyright (C) 2017 public
//
// �޸��ˣ�xiaoquanjie
// ʱ�䣺2017/11/11
//
// �޸��ˣ�xiaoquanjie
// ʱ�䣺
// �޸�˵����
//
// �汾��V1.0.0
//----------------------------------------------------------------*/

#ifndef M_NETIO_TSOCKET_IMPL_INCLUDE
#define M_NETIO_TSOCKET_IMPL_INCLUDE

M_NETIO_NAMESPACE_BEGIN

#define M_SOCKET_READ_SIZE (4*1024)
#define M_SOCKET_PACK_SIZE (400*1024)

inline void MessageHeader::n2h() {
	timestamp = ::ntohl(timestamp);
	size = ::ntohl(size);
}

inline void MessageHeader::h2n() {
	timestamp = ::htonl(timestamp);
	size = ::htonl(size);
}

template<typename T, typename SocketType>
TcpBaseSocket<T, SocketType>::_writerinfo_::_writerinfo_() {
	writing = false;
}

template<typename T, typename SocketType>
TcpBaseSocket<T, SocketType>::_writerinfo_::~_writerinfo_() {
}

template<typename T, typename SocketType>
TcpBaseSocket<T, SocketType>::TcpBaseSocket(BaseNetIo<NetIo>& netio)
	:_netio(netio) {
	_flag = E_STATE_STOP;
	_extdata_func = 0;
	_socket = new SocketType(_netio.GetIoService());
}

template<typename T, typename SocketType>
TcpBaseSocket<T, SocketType>::~TcpBaseSocket() {
	if (_extdata_func)
		_extdata_func(_extdata);
	delete _socket;
}

template<typename T, typename SocketType>
const SocketLib::Tcp::EndPoint& TcpBaseSocket<T, SocketType>::LocalEndpoint()const {
	return _localep;
}

template<typename T, typename SocketType>
const SocketLib::Tcp::EndPoint& TcpBaseSocket<T, SocketType>::RemoteEndpoint()const {
	return _remoteep;
}

template<typename T, typename SocketType>
void TcpBaseSocket<T, SocketType>::Close() {
	/* close�����ã�������ζ�����ӻ����϶Ͽ���socket������е�����ȫ��
	 * �������ŶϿ���
	 */
	_PostClose();
}

template<typename T, typename SocketType>
void TcpBaseSocket<T, SocketType>::_PostClose() {
	SocketLib::ScopedLock scoped_w(_writer.lock);
	_Close();
}

template<typename T, typename SocketType>
void TcpBaseSocket<T, SocketType>::_Close() {
	if (_flag != E_STATE_CLOSE) {
		_flag = E_STATE_STOP;
		if (!_writer.writing) {
			_flag = E_STATE_CLOSE;
			SocketLib::SocketError error;
			_socket->Close(bind_t(&TcpBaseSocket::_CloseHandler, this->shared_from_this()), error);
		}
	}
}

template<typename T, typename SocketType>
bool TcpBaseSocket<T, SocketType>::Send(const SocketLib::Buffer* buffer) {
	if (!buffer)
		return false;
	if (buffer->Length() == 0)
		return false;

	int snd_len = buffer->Size();
	if (snd_len > M_SOCKET_PACK_SIZE) {
		M_NETIO_LOGGER(this->_socket->GetFd() << "|The package is too big to sent in the cache, so been discarded");
		return false;
	}

	SocketLib::ScopedLock scoped_w(_writer.lock);
	if (_flag != E_STATE_START) {
		return false;
	}

	if (_writer.msgbuffer2.Size() + snd_len > _writerinfo_::E_MAX_BUFFER_SIZE) {
		// �ѻ���̫����û�з���ȥ
		M_NETIO_LOGGER(this->_socket->GetFd() << "|There is too much data that is not sent in the cache, so been discarded");
		return false;
	}

	_writer.msgbuffer2.Write((void*)buffer->Raw(), snd_len);
	_TrySendData();
	return true;
}

template<typename T, typename SocketType>
bool TcpBaseSocket<T, SocketType>::Send(const SocketLib::s_byte_t* data, SocketLib::s_uint32_t len) {
	if (len <= 0)
		return false;
	
	if (len > M_SOCKET_PACK_SIZE) {
		M_NETIO_LOGGER(this->_socket->GetFd() << "|The package is too big to sent in the cache, so been discarded");
		return false;
	}

	SocketLib::ScopedLock scoped_w(_writer.lock);
	if (_flag != E_STATE_START)
		return false;

	int snd_len = len + sizeof(MessageHeader);
	if (_writer.msgbuffer2.Size() + snd_len > _writerinfo_::E_MAX_BUFFER_SIZE) {
		// �ѻ���̫����û�з���ȥ
		M_NETIO_LOGGER(this->_socket->GetFd() <<"|There is too much data that is not sent in the cache, so been discarded");
		return false;
	}

	MessageHeader hdr;
	hdr.size = len;
	hdr.timestamp = (unsigned int)time(0);
	hdr.h2n();
	_writer.msgbuffer2.Write(hdr);
	_writer.msgbuffer2.Write((void*)data, len);

	_TrySendData();
	return true;
}

template<typename T, typename SocketType>
bool TcpBaseSocket<T, SocketType>::IsConnected()const {
	return (_flag == E_STATE_START);
}

template<typename T, typename SocketType>
void TcpBaseSocket<T, SocketType>::SetExtData(void* data, void(*func)(void*data)) {
	if (func) {
		_extdata = data;
		_extdata_func = func;
	}
}

template<typename T, typename SocketType>
void TcpBaseSocket<T, SocketType>::SetExtData(void* data) {
	_extdata = data;
}

template<typename T, typename SocketType>
void* TcpBaseSocket<T, SocketType>::GetExtData() {
	return _extdata;
}

template<typename T, typename SocketType>
void TcpBaseSocket<T, SocketType>::SetKeepAlive(SocketLib::s_uint32_t timeo) {
	try {
		SocketLib::Opts::Keepalive kpalive(true);
		_socket->SetOption(kpalive);
#ifdef M_TCP_KEEPCNT
		SocketLib::Opts::TcpKeepCnt kpcnt(3);
		_socket->SetOption(kpcnt);
#endif
#ifdef M_TCP_KEEPINTVL
		SocketLib::Opts::TcpKeepIntvl kpintvl(2);
		_socket->SetOption(kpintvl);
#endif
#ifdef M_TCP_KEEPIDLE
		SocketLib::Opts::TcpKeepIdle kpidle(timeo);
		_socket->SetOption(kpidle);
#endif
	}
	catch (...) {
	}
}

template<typename T, typename SocketType>
void TcpBaseSocket<T, SocketType>::_WriteHandler(SocketLib::s_uint32_t tran_byte, SocketLib::SocketError error) {
	/*
	 *  Ҫע���ֹ_writer.lock���������⡣
	 */
	SocketLib::ScopedLock scoped_w(_writer.lock);
	_writer.writing = false;
	
	if (error) {
		// ����ر�����
		M_NETIO_LOGGER("write handler happend error:"M_ERROR_DESC_STR(error));
		_Close();
	}
	else if (tran_byte <= 0) {
		// �����Ѿ��ر�
		_Close();
	}
	else {
		_writer.msgbuffer1.CutData(tran_byte);
		if (!_TrySendData() && !(_flag == E_STATE_START)) {
			// ���ݷ���������״̬����E_STATE_START������Ҫ�ر�д
			_socket->Shutdown(SocketLib::E_Shutdown_WR, error);
			_Close();
		}
	}
}

template<typename T, typename SocketType>
inline void TcpBaseSocket<T, SocketType>::_CloseHandler() {
	shard_ptr_t<T> ref = this->shared_from_this();
	_netio.OnDisconnected(ref);
}

template<typename T, typename SocketType>
bool TcpBaseSocket<T, SocketType>::_TrySendData() {
	if (_writer.writing)
		return true;

	if (_writer.msgbuffer1.Length() == 0) {
		_writer.msgbuffer1.Swap(_writer.msgbuffer2);
		if (_writer.msgbuffer2.Capacity() >= (1024*1024)) {
			SocketLib::Buffer tmp;
			_writer.msgbuffer2.Swap(tmp);
		}
		_writer.msgbuffer2.Clear();
	}

	if (_writer.msgbuffer1.Length() > 0) {
		SocketLib::SocketError error;
		_socket->AsyncSendSome(bind_t(&TcpBaseSocket::_WriteHandler, this->shared_from_this(), placeholder_1, placeholder_2)
			, _writer.msgbuffer1.Data(), _writer.msgbuffer1.Length(), error);
		if (error) {
			_Close();
		}
		else {
			_writer.writing = true;
		}
		return (!error);
	}

	return false;
}


/////////////////////////////////////////////////////////////////////////////////////////////

template<typename T, typename SocketType>
TcpStreamSocket<T, SocketType>::_readerinfo_::_readerinfo_() {
	g_memset(&curheader, 0, sizeof(curheader));
	readbuf = new SocketLib::s_byte_t[M_SOCKET_READ_SIZE];
	g_memset(readbuf, 0, M_SOCKET_READ_SIZE);
}

template<typename T, typename SocketType>
TcpStreamSocket<T, SocketType>::_readerinfo_::~_readerinfo_() {
	delete[]readbuf;
}

template<typename T, typename SocketType>
void TcpStreamSocket<T, SocketType>::_ReadHandler(SocketLib::s_uint32_t tran_byte, SocketLib::SocketError error) {
	do {
		// ����ر�����
		if (error) {
			M_NETIO_LOGGER("read handler happend error:" << M_ERROR_DESC_STR(error));
			break;
		}
		// �Է��ر�д
		if (tran_byte <= 0)
			break;

		// �ҷ�post�˹ر�
		if (this->_flag != E_STATE_START)
			break;

		if (_CutMsgPack(_reader.readbuf, tran_byte)) {
			_TryRecvData();
			return;
		}
		else {
			// ���ݼ����������Ͽ�����
			this->_socket->Shutdown(SocketLib::E_Shutdown_RD, error);
		}
	} while (false);

	this->_PostClose();
}

template<typename T, typename SocketType>
bool TcpStreamSocket<T, SocketType>::_CutMsgPack(SocketLib::s_byte_t* buf, SocketLib::s_uint32_t tran_byte) {
	// �����ڴ濽���Ǵ˺�������ƹؼ�
	SocketLib::s_uint32_t hdrlen = (SocketLib::s_uint32_t)sizeof(MessageHeader);
	shard_ptr_t<T> ref;
	do
	{
		// ���ͷ������
		SocketLib::s_uint32_t datalen = _reader.msgbuffer.Length();
		if (_reader.curheader.size == 0) {
			if (tran_byte + datalen < hdrlen) {
				_reader.msgbuffer.Write(buf, tran_byte);
				return true;
			}
			else if (datalen >= hdrlen) {
				_reader.msgbuffer.Read(_reader.curheader);
			}
			else {
				_reader.msgbuffer.Write(buf, hdrlen - datalen);
				buf += (hdrlen - datalen);
				tran_byte -= (hdrlen - datalen);
				_reader.msgbuffer.Read(_reader.curheader);
			}

			// convert byte order
			_reader.curheader.n2h();
			
			// check
			if (_reader.curheader.size > (M_SOCKET_PACK_SIZE /*- hdrlen*/))
				return false;
		}

		// copy body data
		datalen = _reader.msgbuffer.Length();
		if (tran_byte + datalen < _reader.curheader.size) {
			_reader.msgbuffer.Write(buf, tran_byte);
			return true;
		}
		else {
			_reader.msgbuffer.Write(buf, _reader.curheader.size - datalen);
			buf += (_reader.curheader.size - datalen);
			tran_byte -= (_reader.curheader.size - datalen);

			// notify
			_reader.curheader.size = 0;
			if (!ref)
				ref = this->shared_from_this();
			this->_netio.OnReceiveData(ref, _reader.msgbuffer);
			_reader.msgbuffer.Clear();
		}
	} while (true);
	return true;
}

template<typename T, typename SocketType>
void TcpStreamSocket<T, SocketType>::_TryRecvData() {
	SocketLib::SocketError error;
	this->_socket->AsyncRecvSome(bind_t(&TcpStreamSocket::_ReadHandler, this->shared_from_this(), placeholder_1, placeholder_2)
		, _reader.readbuf, M_SOCKET_READ_SIZE, error);
	if (error)
		this->_PostClose();
}

template<typename T, typename SocketType>
TcpStreamSocket<T, SocketType>::TcpStreamSocket(BaseNetIo<NetIo>& netio)
	:TcpBaseSocket<T, SocketType>(netio) {
}


M_NETIO_NAMESPACE_END
#endif
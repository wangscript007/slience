/*----------------------------------------------------------------
// Copyright (C) 2017 public
//
// �޸��ˣ�xiaoquanjie
// ʱ�䣺2017/12/02
//
// �޸��ˣ�xiaoquanjie
// ʱ�䣺
// �޸�˵����
//
// �汾��V1.0.0
//----------------------------------------------------------------*/

#ifndef M_NETIO_HCONNECTOR_IMPL_INCLUDE
#define M_NETIO_HCONNECTOR_IMPL_INCLUDE

M_NETIO_NAMESPACE_BEGIN

template<typename ConnectorType>
BaseHConnector<ConnectorType>::BaseHConnector(BaseNetIo<NetIo>& netio)
	:HttpBaseSocket<ConnectorType, SocketLib::TcpConnector<SocketLib::IoService>
	, HttpCliRecvMsg>(netio) {
}

template<typename ConnectorType>
SocketLib::TcpConnector<SocketLib::IoService>& BaseHConnector<ConnectorType>::GetSocket() {
	return (*this->_socket);
}

template<typename ConnectorType>
bool BaseHConnector<ConnectorType>::Connect(const SocketLib::Tcp::EndPoint& ep) {
	try {
		this->_socket->Connect(ep);
		return true;
	}
	catch (SocketLib::SocketError& error) {
		lasterror = error;
		return false;
	}
}

template<typename ConnectorType>
bool BaseHConnector<ConnectorType>::Connect(const std::string& addr, SocketLib::s_uint16_t port) {
	SocketLib::Tcp::EndPoint ep(SocketLib::AddressV4(addr), port);
	return Connect(ep);
}

template<typename ConnectorType>
void BaseHConnector<ConnectorType>::AsyncConnect(const SocketLib::Tcp::EndPoint& ep) {
	try {
		function_t<void(SocketLib::SocketError)> handler = bind_t(&BaseHConnector<ConnectorType>::_ConnectHandler, this,
			placeholder_1, this->shared_from_this());
		this->_socket->AsyncConnect(handler, ep);
	}
	catch (SocketLib::SocketError& error) {
		lasterror = error;
	}
}

template<typename ConnectorType>
void BaseHConnector<ConnectorType>::AsyncConnect(const std::string& addr, SocketLib::s_uint16_t port) {
	SocketLib::Tcp::EndPoint ep(SocketLib::AddressV4(addr), port);
	return AsyncConnect(ep);
}

template<typename ConnectorType>
void BaseHConnector<ConnectorType>::_ConnectHandler(const SocketLib::SocketError& error, HttpConnectorPtr conector) {
	if (error) {
		lasterror = error;
		shard_ptr_t<ConnectorType> ref = this->shared_from_this();
		this->_netio.OnConnected(ref, error);
		return;
	}
	try {
		this->_remoteep = this->_socket->RemoteEndPoint();
		this->_localep = this->_socket->LocalEndPoint();
		this->_flag = E_STATE_START;
		shard_ptr_t<ConnectorType> ref = this->shared_from_this();
		this->_netio.OnConnected(ref, error);
		function_t<void(SocketLib::s_uint32_t, SocketLib::SocketError)> handler =
			bind_t(&BaseHConnector<ConnectorType>::_ReadHandler, ref, placeholder_1, placeholder_2);
		this->_socket->AsyncRecvSome(handler, this->_reader.readbuf, M_SOCKET_READ_SIZE);
	}
	catch (SocketLib::SocketError& err) {
		lasterror = err;
	}
}

M_NETIO_NAMESPACE_END
#endif
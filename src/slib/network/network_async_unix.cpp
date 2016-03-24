#include "network_async_config.h"

#if defined(SLIB_PLATFORM_IS_UNIX)

#include "../../../inc/slib/network/async.h"
#include "../../../inc/slib/core/log.h"

SLIB_NETWORK_NAMESPACE_BEGIN

class _Unix_AsyncTcpSocketInstance : public AsyncTcpSocketInstance
{
public:
	SafeRef<AsyncStreamRequest> m_requestReading;
	SafeRef<AsyncStreamRequest> m_requestWriting;
	sl_uint32 m_sizeWritten;
	
	sl_bool m_flagConnecting;
	
public:
	_Unix_AsyncTcpSocketInstance()
	{
		m_sizeWritten = 0;
		m_flagConnecting = sl_false;
	}
	
	~_Unix_AsyncTcpSocketInstance()
	{
		close();
	}
	
public:
	static Ref<_Unix_AsyncTcpSocketInstance> create(const Ref<Socket>& socket)
	{
		Ref<_Unix_AsyncTcpSocketInstance> ret;
		if (socket.isNotNull()) {
			if (socket->setNonBlockingMode(sl_true)) {
				sl_file handle = (sl_file)(socket->getHandle());
				if (handle != SLIB_FILE_INVALID_HANDLE) {
					ret = new _Unix_AsyncTcpSocketInstance();
					if (ret.isNotNull()) {
						ret->m_socket = socket;
						ret->setHandle(handle);
						return ret;
					}
				}
			}
		}
		return ret;
	}
	
	void close()
	{
		setHandle(SLIB_FILE_INVALID_HANDLE);
		m_socket.setNull();
	}
	
	void processRead(sl_bool flagError)
	{
		Ref<Socket> socket = m_socket;
		if (socket.isNull()) {
			return;
		}
		Ref<AsyncStreamRequest> request = m_requestReading;
		m_requestReading.setNull();
		sl_size nQueue = getReadRequestsCount();
		if (Thread::isNotStoppingCurrent()) {
			if (request.isNull()) {
				if (nQueue > 0) {
					nQueue--;
					popReadRequest(request);
					if (request.isNull()) {
						return;
					}
				} else {
					return;
				}
			}
			sl_int32 n = socket->receive((char*)(request->data), request->size);
			if (n > 0) {
				_onReceive(request.ptr, n, flagError);
			} else if (n < 0) {
				_onReceive(request.ptr, 0, sl_true);
				return;
			} else {
				if (flagError) {
					_onReceive(request.ptr, 0, sl_true);
				} else {
					m_requestReading = request;
				}
				return;
			}
			request.setNull();
		}
	}

	void processWrite(sl_bool flagError)
	{
		Ref<Socket> socket = m_socket;
		if (socket.isNull()) {
			return;
		}
		Ref<AsyncStreamRequest> request = m_requestWriting;
		m_requestWriting.setNull();
		sl_size nQueue = getWriteRequestsCount();
		if (Thread::isNotStoppingCurrent()) {
			if (request.isNull()) {
				if (nQueue > 0) {
					nQueue--;
					popWriteRequest(request);
					if (request.isNull()) {
						return;
					} else {
						m_sizeWritten = 0;
					}
				} else {
					return;
				}
			}
			sl_uint32 size = request->size - m_sizeWritten;
			sl_int32 n = socket->send((char*)(request->data) + m_sizeWritten, size);
			if (n > 0) {
				m_sizeWritten += n;
				if (m_sizeWritten >= request->size) {
					_onSend(request.ptr, request->size, flagError);
				} else {
					m_requestWriting = request;
				}
			} else if (n < 0) {
				_onSend(request.ptr, m_sizeWritten, sl_true);
				return;
			} else {
				if (flagError) {
					_onSend(request.ptr, m_sizeWritten, sl_true);
				} else {
					m_requestWriting = request;
				}
				return;
			}
			request.setNull();
		}
	}
	
	void onOrder()
	{
		Ref<Socket> socket = m_socket;
		if (socket.isNull()) {
			return;
		}
		if (m_flagConnecting) {
			return;
		}
		if (m_flagRequestConnect) {
			m_flagRequestConnect = sl_false;
			if (socket->connect(m_addressRequestConnect)) {
				m_flagConnecting = sl_true;
			} else {
				_onConnect(sl_true);
			}
			return;
		}
		processRead(sl_false);
		processWrite(sl_false);
	}
	
	void onEvent(EventDesc* pev)
	{
		sl_bool flagProcessed = sl_false;
		if (pev->flagIn) {
			processRead(pev->flagError);
			flagProcessed = sl_true;
		}
		if (pev->flagOut) {
			if (m_flagConnecting) {
				m_flagConnecting = sl_false;
				if (pev->flagError) {
					_onConnect(sl_true);
				} else {
					_onConnect(sl_false);
				}
			} else {
				processWrite(pev->flagError);
			}
			flagProcessed = sl_true;
		}
		if (!flagProcessed) {
			if (pev->flagError) {
				if (m_flagConnecting) {
					m_flagConnecting = sl_false;
					_onConnect(sl_true);
				} else {
					processRead(sl_true);
					processWrite(sl_true);
				}
			}
		}
		requestOrder();
	}
};

Ref<AsyncTcpSocket> AsyncTcpSocket::create(const Ref<Socket>& socket, const Ref<AsyncIoLoop>& loop)
{
	Ref<_Unix_AsyncTcpSocketInstance> ret = _Unix_AsyncTcpSocketInstance::create(socket);
	return AsyncTcpSocket::create(ret.ptr, loop);
}

class _Unix_AsyncTcpServerInstance : public AsyncTcpServerInstance
{
public:
	sl_bool m_flagListening;
	
public:
	_Unix_AsyncTcpServerInstance()
	{
		m_flagListening = sl_false;
	}
	
	~_Unix_AsyncTcpServerInstance()
	{
		close();
	}
	
public:
	static Ref<_Unix_AsyncTcpServerInstance> create(const Ref<Socket>& socket, const Ptr<IAsyncTcpServerListener>& listener)
	{
		Ref<_Unix_AsyncTcpServerInstance> ret;
		if (socket.isNotNull()) {
			if (socket->setNonBlockingMode(sl_true)) {
				sl_file handle = (sl_file)(socket->getHandle());
				if (handle != SLIB_FILE_INVALID_HANDLE) {
					ret = new _Unix_AsyncTcpServerInstance();
					if (ret.isNotNull()) {
						ret->m_socket = socket;
						ret->setHandle(handle);
						ret->m_listener = listener;
						return ret;
					}
				}				
			}
		}
		return ret;
	}
	
	void close()
	{
		AsyncTcpServerInstance::close();
		m_socket.setNull();
		setHandle(SLIB_FILE_INVALID_HANDLE);
	}
	
	void onOrder()
	{
		Ref<Socket> socket = m_socket;
		if (socket.isNull()) {
			return;
		}
		while (Thread::isNotStoppingCurrent()) {
			Ref<Socket> socketAccept;
			SocketAddress addr;
			if (socket->accept(socketAccept, addr)) {
				_onAccept(socketAccept, addr);
			} else {
				SocketError err = socket->getLastError();
				if (err != SocketError::WouldBlock) {
					_onError();
				}
				return;
			}
		}
	}
	
	void onEvent(EventDesc* pev)
	{
		if (pev->flagIn) {
			onOrder();
		}
		if (pev->flagError) {
			_onError();
		}
	}
};

Ref<AsyncTcpServer> AsyncTcpServer::create(const Ref<Socket>& socket, const Ptr<IAsyncTcpServerListener>& listener, const Ref<AsyncIoLoop>& loop, sl_bool flagAutoStart)
{
	Ref<_Unix_AsyncTcpServerInstance> ret = _Unix_AsyncTcpServerInstance::create(socket, listener);
	return AsyncTcpServer::create(ret.ptr, loop, flagAutoStart);
}

class _Unix_AsyncUdpSocketInstance : public AsyncUdpSocketInstance
{
public:
	_Unix_AsyncUdpSocketInstance()
	{
	}
	
	~_Unix_AsyncUdpSocketInstance()
	{
		close();
	}
	
public:
	static Ref<_Unix_AsyncUdpSocketInstance> create(const Ref<Socket>& socket, const Ptr<IAsyncUdpSocketListener>& listener, const Memory& buffer)
	{
		Ref<_Unix_AsyncUdpSocketInstance> ret;
		if (socket.isNotNull()) {
			if (socket->setNonBlockingMode(sl_true)) {
				sl_file handle = (sl_file)(socket->getHandle());
				if (handle != SLIB_FILE_INVALID_HANDLE) {
					ret = new _Unix_AsyncUdpSocketInstance();
					if (ret.isNotNull()) {
						ret->m_socket = socket;
						ret->setHandle(handle);
						ret->m_listener = listener;
						ret->m_buffer = buffer;
						return ret;
					}
				}
			}
		}
		return ret;
	}
	
	void close()
	{
		AsyncUdpSocketInstance::close();
		setHandle(SLIB_FILE_INVALID_HANDLE);
		m_socket.setNull();
	}
	
	void onOrder()
	{
		processSend();
		processReceive();
	}
	
	void onEvent(EventDesc* pev)
	{
		if (pev->flagIn) {
			processReceive();
		}
	}
	
	void processSend()
	{
		Ref<Socket> socket = m_socket;
		if (socket.isNull()) {
			return;
		}
		if (!(socket->isOpened())) {
			return;
		}
		while (Thread::isNotStoppingCurrent()) {
			SendRequest request;
			if (m_queueSendRequests.pop(&request)) {
				socket->sendTo(request.addressTo, request.data.getData(), (sl_uint32)(request.data.getSize()));
			} else {
				break;
			}
		}
	}
	
	void processReceive()
	{
		Ref<Socket> socket = m_socket;
		if (socket.isNull()) {
			return;
		}
		if (!(socket->isOpened())) {
			return;
		}
		void* buf = m_buffer.getData();
		sl_uint32 sizeBuf = (sl_uint32)(m_buffer.getSize());
		while (Thread::isNotStoppingCurrent()) {
			SocketAddress addr;
			sl_int32 n = socket->receiveFrom(addr, buf, sizeBuf);
			if (n > 0) {
				_onReceive(addr, n);
			} else {
				break;
			}
		}
	}

};

Ref<AsyncUdpSocket> AsyncUdpSocket::create(const Ref<Socket>& socket, const Ptr<IAsyncUdpSocketListener>& listener, sl_uint32 packetSize, const Ref<AsyncIoLoop>& loop, sl_bool flagAutoStart)
{
	Memory buffer = Memory::create(packetSize);
	if (buffer.isNotEmpty()) {
		Ref<_Unix_AsyncUdpSocketInstance> ret = _Unix_AsyncUdpSocketInstance::create(socket, listener, buffer);
		return AsyncUdpSocket::create(ret.ptr, loop, flagAutoStart);
	}
	return Ref<AsyncUdpSocket>::null();
}

SLIB_NETWORK_NAMESPACE_END

#endif

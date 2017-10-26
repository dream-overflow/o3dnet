/**
 * @file proxymessages.cpp
 * @brief 
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2001-12-25
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#include "o3d/net/proxymessages.h"
#include "o3d/net/netbuffer.h"

#include "o3d/net/proxyclient.h"
#include "o3d/net/proxyserver.h"

#include <o3d/core/debug.h>

using namespace o3d;
using namespace o3d::net;

NetMessage *ProxyChallengeOut::writeToBuffer(NetBuffer *buffer)
{
    buffer->writeInt32(m_version);
    buffer->write(m_challenge, 16);

    return nullptr;
}

NetMessage* ProxyChallengeIn::readFromBuffer(NetBuffer* buffer)
{
    m_version = buffer->readInt32();
    buffer->read(m_challenge, 16);

    return nullptr;
}

void ProxyChallengeIn::run(void *context)
{
    // run on proxy client side
    ProxyClient *proxyClient = (ProxyClient*)context;

    // check the version
    if (m_version != proxyClient->getVersion())
        O3D_ERROR(E_RunMessage("Unconsistent proxy client and server version"));

    // send the certificate
    ProxyCertificateOut *cert = new ProxyCertificateOut;
    cert->setCertificate(proxyClient->getCertificate());

    proxyClient->send(cert);
}

NetMessage *ProxyCertificateOut::writeToBuffer(NetBuffer *buffer)
{
    buffer->write((const UInt8*)m_certificate.getData(), m_certificate.getSizeInBytes());

    return nullptr;
}

NetMessage *ProxyCertificateIn::readFromBuffer(NetBuffer *buffer)
{
    m_certificate.allocate(m_messageDataSize);
    buffer->read((UInt8*)m_certificate.getData(), m_messageDataSize);

    return nullptr;
}

void ProxyCertificateIn::run(void *context)
{
    // run on proxy server side
    ProxyServerSession *session = (ProxyServerSession*)context;

    UInt32 size = m_certificate.getSizeInBytes();
    if (size != session->getProxyServer()->getCertificate().getSizeInBytes())
    {
        session->cancel();
        O3D_WARNING("Cancel proxy session because of invalid certificate");

        return;
    }

    if (memcmp(
                m_certificate.getData(),
                session->getProxyServer()->getCertificate().getData(),
                size) != 0)
    {
        session->cancel();
        O3D_WARNING("Cancel proxy session because of invalid certificate");

        return;
    }

    session->setValid();

    O3D_MESSAGE("Validate proxy session");
}


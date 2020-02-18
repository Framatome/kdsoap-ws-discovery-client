/* Copyright (C) 2019 Casper Meijn <casper@meijn.net>
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "wsdiscoveryclient.h"

#include "loggingcategory.h"
#include "wsdiscoverytargetservice.h"

#include <KDSoapClient/KDSoapMessage>
#include <KDSoapClient/KDSoapMessageAddressingProperties>
#include <KDSoapClient/KDSoapUdpClient>
#include <QHostAddress>
#include <QTimer>
#include <QUrl>
#include <QUuid>

#include "wsdl_wsdd-discovery-1.h"
#include "wsdl_ws-discovery.h"

static const int DISCOVERY_PORT = 3702;
static const QHostAddress DISCOVERY_ADDRESS_IPV4 = QHostAddress(QStringLiteral("239.255.255.250"));
static const QHostAddress DISCOVERY_ADDRESS_IPV6 = QHostAddress(QStringLiteral("FF02::C"));

WSDiscoveryClient::WSDiscoveryClient(QObject *parent) :
    QObject(parent)
{
    m_soapUdpClient = new KDSoapUdpClient(this);
    connect(m_soapUdpClient, &KDSoapUdpClient::receivedMessage, this, &WSDiscoveryClient::receivedMessage);
}

WSDiscoveryClient::~WSDiscoveryClient() = default;

void WSDiscoveryClient::start()
{
    bool rc = m_soapUdpClient->bind(DISCOVERY_PORT, QAbstractSocket::ShareAddress);
    if (!rc) {
        qWarning() << "start()->KDSoapUdpClient::bind() returned false when binding to port" << DISCOVERY_PORT;
    }
}

void WSDiscoveryClient::sendProbe(const QList<KDQName> &typeList, const QList<QUrl> &scopeList)
{
    WSDiscovery200504::TNS__ProbeType probe;
    if(!typeList.isEmpty()) {
        WSDiscovery200504::TNS__QNameListType types;
        types.setEntries(typeList);
        probe.setTypes(types);
    }
    if(!scopeList.isEmpty()) {
        WSDiscovery200504::TNS__UriListType scopeValues;
        scopeValues.setEntries(QUrl::toStringList(scopeList));
        WSDiscovery200504::TNS__ScopesType scopes;
        scopes.setValue(scopeValues);
        probe.setScopes(scopes);
    }

    KDSoapMessage message;
    message = probe.serialize(QStringLiteral("Probe"));
    message.setUse(KDSoapMessage::LiteralUse);
    message.setNamespaceUri(QStringLiteral("http://schemas.xmlsoap.org/ws/2005/04/discovery"));

    KDSoapMessageAddressingProperties addressing;
    addressing.setAddressingNamespace(KDSoapMessageAddressingProperties::Addressing200408);
    addressing.setAction(QStringLiteral("http://schemas.xmlsoap.org/ws/2005/04/discovery/Probe"));
    addressing.setMessageID(QStringLiteral("urn:uuid:") + QUuid::createUuid().toString().remove("{").remove("}"));
    addressing.setDestination(QStringLiteral("urn:schemas-xmlsoap-org:ws:2005:04:discovery"));
    addressing.setReplyEndpointAddress(KDSoapMessageAddressingProperties::predefinedAddressToString(KDSoapMessageAddressingProperties::Anonymous));
    message.setMessageAddressingProperties(addressing);

    auto rc = m_soapUdpClient->sendMessage(message, KDSoapHeaders(), DISCOVERY_ADDRESS_IPV4, DISCOVERY_PORT);
    if (!rc) {
        qWarning() << "sendProbe()->KDSoapUdpClient::sendMessage() returned false for" << DISCOVERY_ADDRESS_IPV4 << "on port" << DISCOVERY_PORT;
    }
    rc = m_soapUdpClient->sendMessage(message, KDSoapHeaders(), DISCOVERY_ADDRESS_IPV6, DISCOVERY_PORT);
    if (!rc) {
        qWarning() << "sendProbe()->KDSoapUdpClient::sendMessage() returned false for" << DISCOVERY_ADDRESS_IPV6 << "on port" << DISCOVERY_PORT;
    }
}

void WSDiscoveryClient::sendResolve(const QString &endpointReferenceString)
{
    WSDiscovery200504::TNS__ResolveType resolve;

    WSDiscovery200504::WSA__AttributedURI endpointReferenceAddress;
    endpointReferenceAddress.setValue(endpointReferenceString);
    WSDiscovery200504::WSA__EndpointReferenceType endpointReference;
    endpointReference.setAddress(endpointReferenceAddress);
    resolve.setEndpointReference(endpointReference);

    KDSoapMessage message;
    message = resolve.serialize(QStringLiteral("Resolve"));
    message.setUse(KDSoapMessage::LiteralUse);
    message.setNamespaceUri(QStringLiteral("http://schemas.xmlsoap.org/ws/2005/04/discovery"));

    KDSoapMessageAddressingProperties addressing;
    addressing.setAddressingNamespace(KDSoapMessageAddressingProperties::Addressing200408);
    addressing.setAction(QStringLiteral("http://schemas.xmlsoap.org/ws/2005/04/discovery/Resolve"));
    addressing.setMessageID(QStringLiteral("urn:uuid:") + QUuid::createUuid().toString().remove("{").remove("}"));
    addressing.setDestination(QStringLiteral("urn:schemas-xmlsoap-org:ws:2005:04:discovery"));
    addressing.setReplyEndpointAddress(KDSoapMessageAddressingProperties::predefinedAddressToString(KDSoapMessageAddressingProperties::Anonymous));
    message.setMessageAddressingProperties(addressing);

    auto rc = m_soapUdpClient->sendMessage(message, KDSoapHeaders(), DISCOVERY_ADDRESS_IPV4, DISCOVERY_PORT);
    if (!rc) {
        qWarning() << "sendResolve()->KDSoapUdpClient::sendMessage() returned false for" << DISCOVERY_ADDRESS_IPV4 << "on port" << DISCOVERY_PORT;
    }
    rc = m_soapUdpClient->sendMessage(message, KDSoapHeaders(), DISCOVERY_ADDRESS_IPV6, DISCOVERY_PORT);
    if (!rc) {
        qWarning() << "sendResolve()->KDSoapUdpClient::sendMessage() returned false for" << DISCOVERY_ADDRESS_IPV6 << "on port" << DISCOVERY_PORT;
    }
}

void WSDiscoveryClient::receivedMessage(const KDSoapMessage &replyMessage, const KDSoapHeaders &replyHeaders, const QHostAddress &senderAddress, quint16 senderPort)
{
    Q_UNUSED(replyHeaders);
    Q_UNUSED(senderAddress);
    Q_UNUSED(senderPort);
    if(replyMessage.messageAddressingProperties().action() == QStringLiteral("http://schemas.xmlsoap.org/ws/2005/04/discovery/Probe")) {
        // NO-OP
    } else if(replyMessage.messageAddressingProperties().action() == QStringLiteral("http://schemas.xmlsoap.org/ws/2005/04/discovery/Resolve")) {
        // NO-OP
    } else if(replyMessage.messageAddressingProperties().action() == QStringLiteral("http://schemas.xmlsoap.org/ws/2005/04/discovery/ResolveMatches")) {
        WSDiscovery200504::TNS__ResolveMatchesType resolveMatches;
        resolveMatches.deserialize(replyMessage);

        auto resolveMatch = resolveMatches.resolveMatch();
        const QString& endpointReference = resolveMatch.endpointReference().address();
        QSharedPointer<WSDiscoveryTargetService> service = m_targetServiceMap.value(endpointReference);
        if(service.isNull()) {
            service = QSharedPointer<WSDiscoveryTargetService>::create(endpointReference);
            m_targetServiceMap.insert(endpointReference, service);
        }
        service->setTypeList(resolveMatch.types().entries());
        service->setScopeList(QUrl::fromStringList(resolveMatch.scopes().value().entries()));
        service->setXAddrList(QUrl::fromStringList(resolveMatch.xAddrs().entries()));
        service->updateLastSeen();
        emit resolveMatchReceived(service);
    } else if(replyMessage.messageAddressingProperties().action() == QStringLiteral("http://schemas.xmlsoap.org/ws/2005/04/discovery/ProbeMatches")) {
        WSDiscovery200504::TNS__ProbeMatchesType probeMatches;
        probeMatches.deserialize(replyMessage);

        for(const WSDiscovery200504::TNS__ProbeMatchType& probeMatch : probeMatches.probeMatch()) {
            const QString& endpointReference = probeMatch.endpointReference().address();
            QSharedPointer<WSDiscoveryTargetService> service = m_targetServiceMap.value(endpointReference);
            if(service.isNull()) {
                service = QSharedPointer<WSDiscoveryTargetService>::create(endpointReference);
                m_targetServiceMap.insert(endpointReference, service);
            }
            service->setTypeList(probeMatch.types().entries());
            service->setScopeList(QUrl::fromStringList(probeMatch.scopes().value().entries()));
            service->setXAddrList(QUrl::fromStringList(probeMatch.xAddrs().entries()));
            service->updateLastSeen();
            emit probeMatchReceived(service);
        }
    } else {
        qCDebug(KDSoapWSDiscoveryClient) << "Received message with unknown action:" << replyMessage.messageAddressingProperties().action();
    }
}


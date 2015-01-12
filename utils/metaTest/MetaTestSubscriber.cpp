/*************************************************************************
 * Copyright (c) 2015 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file MetaTestSubscriber.cpp
 *
 */

#include "MetaTestSubscriber.h"
#include "fastrtps/fastrtps_all.h"

#include "types/MetaTestTypes.h"
//TESTS INCLUDE FILES

#include "../useTests/RTPSTest_as_socket/TestReaderSocket.h"
#include "../useTests/RTPSTest_registered/TestReaderRegistered.h"

namespace eprosima {

MetaTestSubscriber::MetaTestSubscriber():mp_par(nullptr),
		mp_pub(nullptr),
		mp_sub(nullptr)
{
	// TODO Auto-generated constructor stub

}

MetaTestSubscriber::~MetaTestSubscriber() {
	// TODO Auto-generated destructor stub
	Domain::removeParticipant(mp_par);
}

bool MetaTestSubscriber::init()
{
	ParticipantAttributes Patt;
	Patt.rtps.builtin.domainId = 50;
	Patt.rtps.builtin.leaseDuration = c_TimeInfinite;
	Patt.rtps.setName("MetaSubscriber");

	mp_par = Domain::createParticipant(Patt);
	if(mp_par == nullptr)
		return false;
	Domain::registerType(mp_par,&m_dataType);

	PublisherAttributes Wparam;
	Wparam.topic.topicName = "metaTest_S2P";
	Wparam.topic.topicDataType = "MetaTestType";

	mp_pub = Domain::createPublisher(mp_par,Wparam,&m_publisten);


	SubscriberAttributes Rparam;
	Rparam.topic.topicName = "metaTest_P2S";
	Rparam.topic.topicDataType = "MetaTestType";

	mp_sub = Domain::createSubscriber(mp_par,Rparam,&m_sublisten);

	if(mp_par == nullptr ||mp_pub == nullptr || mp_sub == nullptr)
		return false;
	logUser("MetaTest Subscriber initialized");
	return true;
}

void MetaTestSubscriber::run()
{
	MetaTestType testinfo;
	SampleInfo_t sampleinfo;
	while(1)
	{
		mp_sub->waitForUnreadMessage();
		if(mp_sub->takeNextData(&testinfo,&sampleinfo))
		{
			switch(testinfo.kind())
			{
			case T_RTPS_SOCKET:
				t_rtps_socket(testinfo);
				break;
			case T_RTPS_REG:
				t_rtps_registered(testinfo);
				break;
			case STOP_ALL_TESTS:
				return;
			}
		}
	}
}

void MetaTestSubscriber::t_rtps_registered(MetaTestType& testinfo)
{
	logUser("Starting TEST RTPS REGISTERED");
	TestReaderRegistered trreg;
	SampleInfo_t sampleinfo;
	if(trreg.init() && trreg.reg())
	{
		while (trreg.m_listener.n_matched==0)
		{
			eClock::my_sleep(100);
		}
		testinfo.status(T_SUB_READY);
		mp_pub->write(&testinfo);
		mp_sub->waitForUnreadMessage();
		if(mp_sub->takeNextData(&testinfo,&sampleinfo))
		{
			if(testinfo.status() == T_PUB_FINISH)
			{
				logUser("Publisher has finished (" << testinfo.samples() << " samples send), I received: "
						<<trreg.m_listener.n_received);
				if(trreg.m_listener.n_received == testinfo.samples())
				{
					testinfo.status(T_SUB_OK);
					mp_pub->write(&testinfo);
					return;
				}
				else
					testinfo.comment("Bad samples number received");
			}
			else
				testinfo.comment("Received bad finish code");
		}
		else
			testinfo.comment("Meta Error");
	}
	else
		testinfo.comment("Subscriber Initialization failure");
	testinfo.status(T_SUB_FAILED);
	mp_pub->write(&testinfo);
	return;
}



void MetaTestSubscriber::t_rtps_socket(MetaTestType& testinfo)
{
	logUser("Starting TEST RTPS SOCKET");
	TestReaderSocket tsocket;
	SampleInfo_t sampleinfo;
	if(tsocket.init(testinfo.ip_string(),(uint32_t)testinfo.ip_port()))
	{
		testinfo.status(T_SUB_READY);
		mp_pub->write(&testinfo);
		mp_sub->waitForUnreadMessage();
		if(mp_sub->takeNextData(&testinfo,&sampleinfo))
		{
			if(testinfo.status() == T_PUB_FINISH)
			{
				logUser("Publisher has finished (" << testinfo.samples() << " samples send), I received: "
						<<tsocket.m_listener.m_received);
				if(tsocket.m_listener.m_received == testinfo.samples())
				{
					testinfo.status(T_SUB_OK);
					mp_pub->write(&testinfo);
					return;
				}
				else
					testinfo.comment("Bad samples number received");
			}
			else
				testinfo.comment("Received bad finish code");
		}
		else
			testinfo.comment("Meta Error");
	}
	else
		testinfo.comment("Subscriber Initialization failure");
	testinfo.status(T_SUB_FAILED);
	mp_pub->write(&testinfo);
	return;
}



} /* namespace eprosima */

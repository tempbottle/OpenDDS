// -*- C++ -*-
//
#include "DataReaderListener.h"
#include "MessengerTypeSupportC.h"
#include "MessengerTypeSupportImpl.h"
#include <dds/DCPS/Service_Participant.h>
#include <ace/streams.h>
#include "tests/Utils/ExceptionStreams.h"

using namespace Messenger;
using namespace std;

DataReaderListenerImpl::DataReaderListenerImpl()
  : ok_(true), num_reads_(0), last_non_durable_(0)
{
  ACE_DEBUG((LM_INFO, "(%P|%t) DataReaderListnerImpl[%@]\n", this));
}

DataReaderListenerImpl::~DataReaderListenerImpl ()
{
}

void DataReaderListenerImpl::on_data_available(DDS::DataReader_ptr reader)
{
  DDS::DataReaderQos qos;
  reader->get_qos(qos);
  const bool durable = qos.durability.kind > DDS::VOLATILE_DURABILITY_QOS;

  try {
    MessageDataReader_var message_dr = MessageDataReader::_narrow(reader);
    if (CORBA::is_nil(message_dr)) {
      ACE_ERROR((LM_ERROR, ACE_TEXT("(%P|%t) read: _narrow failed.\n")));
      exit(1);
    }

    Messenger::Message message;
    DDS::SampleInfo si;
    DDS::ReturnCode_t status = message_dr->take_next_sample(message, si);

    if (si.valid_data) {
      if (status == DDS::RETCODE_OK) {

        if (durable) ++num_reads_;

        ACE_DEBUG((LM_INFO, "(%P|%t) DataReaderListenerImpl[%@]::on_data_available - %C instance: %d count = %d\n", this,
              durable ? "Durable " : "Volatile", message.subject_id, message.count));
        if (durable && (message.count != num_reads_)) {
          ACE_DEBUG((LM_INFO, "(%P|%t) ERROR: durable reader received out-of-order data (msg count = %d and num_reads = %d)\n", message.count, num_reads_));
          ok_ = false;
        } else if (!durable) {
          if (last_non_durable_ && message.count != last_non_durable_ + 1) {
            ACE_DEBUG((LM_INFO, "(%P|%t) ERROR: volatile reader received out-of-order data\n"));
            ok_ = false;
          }
          last_non_durable_ = message.count;
        }
      } else if (status == DDS::RETCODE_NO_DATA) {
        ACE_ERROR((LM_ERROR, ACE_TEXT("(%P|%t) ERROR: reader received DDS::RETCODE_NO_DATA!\n")));
        ok_ = false;
      } else {
        ACE_ERROR((LM_ERROR, ACE_TEXT("(%P|%t) ERROR: read Message: Error: %C\n"), status));
        ok_ = false;
      }
    }
  } catch (CORBA::Exception& e) {
    ACE_ERROR((LM_ERROR, ACE_TEXT("(%P|%t) Exception caught in read:\n%C\n"), e._info().c_str()));
    exit(1);
  }
}

void DataReaderListenerImpl::on_requested_deadline_missed (
    DDS::DataReader_ptr,
    const DDS::RequestedDeadlineMissedStatus &)
{
  ACE_ERROR((LM_ERROR, ACE_TEXT("(%P|%t) DataReaderListenerImpl::on_requested_deadline_missed\n")));
}

void DataReaderListenerImpl::on_requested_incompatible_qos (
    DDS::DataReader_ptr,
    const DDS::RequestedIncompatibleQosStatus &)
{
  ACE_ERROR((LM_ERROR, ACE_TEXT("(%P|%t) DataReaderListenerImpl::on_requested_incompatible_qos\n")));
}

void DataReaderListenerImpl::on_liveliness_changed (
    DDS::DataReader_ptr,
    const DDS::LivelinessChangedStatus &)
{
  ACE_ERROR((LM_ERROR, ACE_TEXT("(%P|%t) DataReaderListenerImpl::on_liveliness_changed\n")));
}

void DataReaderListenerImpl::on_subscription_matched (
    DDS::DataReader_ptr,
    const DDS::SubscriptionMatchedStatus &)
{
  ACE_DEBUG((LM_INFO, "(%P|%t) DataReaderListnerImpl::on_subscription_matched\n"));
}

void DataReaderListenerImpl::on_sample_rejected(
    DDS::DataReader_ptr,
    const DDS::SampleRejectedStatus&)
{
  ACE_ERROR((LM_ERROR, ACE_TEXT("(%P|%t) DataReaderListenerImpl::on_sample_rejected\n")));
}

void DataReaderListenerImpl::on_sample_lost(
  DDS::DataReader_ptr,
  const DDS::SampleLostStatus&)
{
  ACE_ERROR((LM_ERROR, ACE_TEXT("(%P|%t) DataReaderListenerImpl::on_sample_lost\n")));
}

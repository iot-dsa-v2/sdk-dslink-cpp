#ifndef DSA_SDK_STREAM_CALLBACKS_H
#define DSA_SDK_STREAM_CALLBACKS_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <functional>

namespace dsa {
class IncomingSubscribeStream;
class SubscribeResponseMessage;
class IncomingListStream;
class ListResponseMessage;
class IncomingInvokeStream;
class InvokeResponseMessage;
class IncomingSetStream;
class SetResponseMessage;

class InvokeRequestMessage;
class SetRequestMessage;

typedef std::function<void(IncomingSubscribeStream&,
                           ref_<const SubscribeResponseMessage>&&)>
    IncomingSubscribeStreamCallback;

typedef std::function<void(IncomingListStream&,
                           ref_<const ListResponseMessage>&&)>
    IncomingListStreamCallback;

typedef std::function<void(IncomingInvokeStream&,
                           ref_<const InvokeResponseMessage>&&)>
    IncomingInvokeStreamCallback;

typedef std::function<void(IncomingSetStream&,
                           ref_<const SetResponseMessage>&&)>
    IncomingSetStreamCallback;
}

#endif  // DSA_SDK_STREAM_CALLBACKS_H

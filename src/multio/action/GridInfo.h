/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Domokos Sarmany

/// @date Aug 2020

#ifndef multio_server_actions_GridInfo_H
#define multio_server_actions_GridInfo_H

#include "multio/message/Message.h"

namespace multio {
namespace action {

class GridInfo {
public:
    GridInfo();

    void setSubtype(const std::string& subtype);
    void setLatitudes(message::Message msg);
    void setLongitudes(message::Message msg);

    const message::Message& latitudes() const;
    const message::Message& longitudes() const;

    bool computeHashIfCan();
    const unsigned char* hash() const;

private:

    message::Message latitudes_;
    message::Message longitudes_;
    std::string gridSubtype_;

    const unsigned char* hash_ = nullptr;
};

}  // namespace action
}  // namespace multio

#endif

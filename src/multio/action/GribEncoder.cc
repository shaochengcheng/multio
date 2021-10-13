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

#include "GribEncoder.h"

#include <cstring>
#include <iomanip>
#include <iostream>

#include "eckit/exception/Exceptions.h"
#include "eckit/log/Log.h"
#include "multio/LibMultio.h"
#include "multio/action/GridInfo.h"


namespace multio {
namespace action {

using message::Message;
using message::Peer;

namespace  {
// TODO: perhaps move this to Mappings as that is already a singleton
std::map<std::string, std::unique_ptr<GridInfo>>& grids() {
    static std::map<std::string, std::unique_ptr<GridInfo>> grids_;
    return grids_;
}

const std::map<const std::string, const long> ops_to_code{
    {"average", 0}, {"accumulate", 1}, {"maximum", 2}, {"minimum", 3}, {"stddev", 6}};

const std::map<const std::string, const std::string> category_to_levtype{
    {"ocean-grid-coordinate", "oceanSurface"},
    {"ocean-2d", "oceanSurface"},
    {"ocean-3d", "oceanModelLevel"}};

const std::map<const std::string, const long> type_of_generating_process{
    {"an", 0}, {"in", 1}, {"fc", 2}, {"pf", 4}};

}  // namespace

GribEncoder::GribEncoder(codes_handle* handle, const std::string& gridType) :
    metkit::grib::GribHandle{handle}, gridType_{gridType} {
    for (auto const& subtype : {"T grid", "U grid", "V grid", "W grid", "F grid"}) {
        grids().insert(std::make_pair(subtype, std::unique_ptr<GridInfo>{new GridInfo{}}));
    }
}

bool GribEncoder::gridInfoReady(const std::string& subtype) const {
    return grids().at(subtype)->hashExists();
}

bool GribEncoder::setGridInfo(message::Message msg) {
    ASSERT(not gridInfoReady(msg.domain())); // Panic check during development

    ASSERT(coordSet_.find(msg.metadata().getString("nemoParam")) != end(coordSet_));

    grids().at(msg.domain())->setSubtype(msg.domain());

    if (msg.metadata().getString("nemoParam").substr(0, 3) == "lat") {
        grids().at(msg.domain())->setLatitudes(msg);
    }

    if (msg.metadata().getString("nemoParam").substr(0, 3) == "lon") {
        grids().at(msg.domain())->setLongitudes(msg);
    }

    return grids().at(msg.domain())->computeHashIfCan();
}

void GribEncoder::setOceanMetadata(const message::Metadata& metadata) {

    // Set run-specific metadata

    setValue("expver", metadata.getSubConfiguration("run").getString("expver"));
    setValue("class", metadata.getSubConfiguration("run").getString("class"));
    setValue("stream", metadata.getSubConfiguration("run").getString("stream"));
    setValue("type", metadata.getSubConfiguration("run").getString("type"));

    // setCommonMetadata
    setValue("step", metadata.getLong("step"));

    if (metadata.getSubConfiguration("run").has("dateOfAnalysis")) {
      auto dateOfAnalysis = metadata.getSubConfiguration("run").getString("dateOfAnalysis");
      setValue("yearOfAnalysis", std::stol(dateOfAnalysis.substr(0, 4)));
      setValue("monthOfAnalysis", std::stol(dateOfAnalysis.substr(4, 2)));
      setValue("dayOfAnalysis", std::stol(dateOfAnalysis.substr(6, 2)));
    }

    if (metadata.getSubConfiguration("run").has("number")) {
      setValue("number", metadata.getSubConfiguration("run").getLong("number"));
    }

    setValue("year", metadata.getLong("date") / 10000);
    setValue("month", (metadata.getLong("date") % 10000) / 100);
    setValue("day", metadata.getLong("date") % 100);

    // Statistics field
    if (metadata.has("operation") and metadata.getString("operation") != "instant") {
        setValue("typeOfStatisticalProcessing", ops_to_code.at(metadata.getString("operation")));
        setValue("stepRange", metadata.getString("stepRange"));
        setValue("indicatorOfUnitForTimeIncrement", 13l); // always seconds
        setValue("timeIncrement", metadata.getLong("timeStep"));
    }

    // setDomainDimensions
    setValue("numberOfDataPoints", metadata.getLong("globalSize"));
    setValue("numberOfValues", metadata.getLong("globalSize"));

    // Setting parameter ID
    setValue("paramId", metadata.getLong("param"));

    setValue("typeOfLevel", metadata.getString("typeOfLevel"));
    auto category = metadata.getString("category");
    if (category == "ocean-3d") {
        auto level = metadata.getLong("level");
        ASSERT(level > 0);
        setValue("scaledValueOfFirstFixedSurface", level);
        setValue("scaledValueOfSecondFixedSurface", level + 1);
    }

    // Set ocean grid information
    setValue("unstructuredGridType", gridType_);

    const auto& gridSubtype = metadata.getString("gridSubtype");
    setValue("unstructuredGridSubtype", gridSubtype.substr(0, 1));

    setValue("uuidOfHGrid", grids().at(gridSubtype)->hashValue());
}

void GribEncoder::setCoordMetadata(const message::Metadata& metadata) {
    // Set run-specific metadata
    setValue("expver", metadata.getSubConfiguration("run").getString("expver"));
    setValue("class", metadata.getSubConfiguration("run").getString("class"));
    setValue("stream", metadata.getSubConfiguration("run").getString("stream"));
    const std::string& type = metadata.getSubConfiguration("run").getString("type");
    setValue("type", type);
    setValue("typeOfGeneratingProcess", type_of_generating_process.at(type));

    setValue("date", metadata.getLong("date"));

    // setDomainDimensions
    setValue("numberOfDataPoints", metadata.getLong("globalSize"));
    setValue("numberOfValues", metadata.getLong("globalSize"));

    // Setting parameter ID
    setValue("paramId", metadata.getLong("param"));

    setValue("typeOfLevel", metadata.getString("typeOfLevel"));

    // Set ocean grid information
    setValue("unstructuredGridType", gridType_);

    const auto& gridSubtype = metadata.getString("gridSubtype");
    setValue("unstructuredGridSubtype", gridSubtype.substr(0, 1));

    setValue("uuidOfHGrid", grids().at(gridSubtype)->hashValue());
}

void GribEncoder::setValue(const std::string& key, long value) {
    LOG_DEBUG_LIB(LibMultio) << "*** Setting value " << value << " for key " << key << std::endl;
    CODES_CHECK(codes_set_long(raw(), key.c_str(), value), NULL);
}

// void GribEncoder::setValue(const std::string& key, unsigned long value) {
//     eckit::Log::info() << "*** Setting value " << value << " for key " << key << std::endl;
//     // LOG_DEBUG_LIB(LibMultio) << "*** Setting value " << value << " for key " << key << std::endl;
//     CODES_CHECK(codes_set_long(raw(), key.c_str(), value), NULL);
// }

void GribEncoder::setValue(const std::string& key, double value) {
    LOG_DEBUG_LIB(LibMultio) << "*** Setting value " << value << " for key " << key << std::endl;
    CODES_CHECK(codes_set_double(raw(), key.c_str(), value), NULL);
}

void GribEncoder::setValue(const std::string& key, const std::string& value) {
    LOG_DEBUG_LIB(LibMultio) << "*** Setting value " << value << " for key " << key << std::endl;
    size_t sz = value.size();
    CODES_CHECK(codes_set_string(raw(), key.c_str(), value.c_str(), &sz), NULL);
}

void GribEncoder::setValue(const std::string& key, const unsigned char* value) {
    std::ostringstream oss;
    for (int i = 0; i < DIGEST_LENGTH; ++i) {
        oss << ((i == 0) ? "" : "-") << std::hex << std::setfill('0') << std::setw(2)
            << static_cast<short>(value[i]);
    }
    LOG_DEBUG_LIB(LibMultio) << "*** Setting value " << oss.str() << " for key " << key
                             << std::endl;
    size_t sz = DIGEST_LENGTH;
    CODES_CHECK(codes_set_bytes(raw(), key.c_str(), value, &sz), NULL);
}

message::Message GribEncoder::encodeLatitudes(const std::string& subtype) {
    auto msg = grids().at(subtype)->latitudes();

    setCoordMetadata(msg.metadata());

    return setFieldValues(msg);
}

message::Message GribEncoder::encodeLongitudes(const std::string& subtype) {
    auto msg = grids().at(subtype)->longitudes();

    setCoordMetadata(msg.metadata());

    return setFieldValues(msg);
}

message::Message GribEncoder::encodeField(const message::Message& msg) {
        setOceanMetadata(msg.metadata());
        return setFieldValues(msg);
}

message::Message GribEncoder::encodeField(const message::Metadata& md, const double* data,
                                          size_t sz) {
    setOceanMetadata(md);
    return setFieldValues(data, sz);
}

message::Message GribEncoder::setFieldValues(const message::Message& msg) {
    auto beg = reinterpret_cast<const double*>(msg.payload().data());
    this->setDataValues(beg, msg.globalSize());

    eckit::Buffer buf{this->length()};
    this->write(buf);

    return Message{Message::Header{Message::Tag::Grib, Peer{}, Peer{}}, std::move(buf)};
}

message::Message GribEncoder::setFieldValues(const double* values, size_t count) {
    this->setDataValues(values, count);

    eckit::Buffer buf{this->length()};
    this->write(buf);

    return Message{Message::Header{Message::Tag::Grib, Peer{}, Peer{}}, std::move(buf)};
}

}  // namespace action
}  // namespace multio

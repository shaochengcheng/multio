/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Tiago Quintino
/// @date Dec 2015


#ifndef multio_fdb4_FDB4Sink_H
#define multio_fdb4_FDB4Sink_H

#include <iosfwd>
#include <string>
#include <vector>

#include "eckit/memory/NonCopyable.h"
#include "eckit/io/Length.h"

#include "multio/DataSink.h"

//----------------------------------------------------------------------------------------------------------------------

class FDB4Sink : public multio::DataSink {

public:

    FDB4Sink(const eckit::Configuration& config);

    virtual ~FDB4Sink();

    virtual void write(eckit::DataBlobPtr blob);

    ///
    /// LEGACY INTERFACE TO REMOVE AFTER IFS CHANGED TO SIMPLE WRITE() INTERFACE
    ///

    virtual void iopenfdb(const std::string& name, int& fdbaddr, const std::string& mode);
    virtual void iinitfdb();
    virtual void iclosefdb(int fdbaddr);

    virtual void isetcommfdb(int rank);
    virtual void isetrankfdb(int fdbaddr, int rank);
    virtual void iset_fdb_root(int fdbaddr, const std::string& name);

    virtual void iflushfdb(int fdbaddr);

    virtual void isetfieldcountfdb(int fdbaddr, int all_ranks, int this_rank);
    virtual void isetvalfdb(int fdbaddr, const std::string& name, const std::string& value);

    // virtual int ireadfdb(void *data, int *words);
    virtual void iwritefdb(int fdbaddr, eckit::DataBlobPtr blob);

protected:

    virtual void print(std::ostream&) const;

private:

    friend std::ostream &operator<<(std::ostream &s, const FDB4Sink &p) {
        p.print(s);
        return s;
    }

};

//----------------------------------------------------------------------------------------------------------------------

#endif


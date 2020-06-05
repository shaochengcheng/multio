
#ifndef multio_server_actions_TemporalStatistics_H
#define multio_server_actions_TemporalStatistics_H

#include <map>
#include <string>
#include <vector>

#include "eckit/types/Date.h"

#include "message/Message.h"

namespace multio {
namespace action {

class TemporalStatistics {
public:
    TemporalStatistics(const std::vector<std::string> operations, long fld_sz);
    virtual ~TemporalStatistics() = default;

protected:
    std::map<std::string, std::vector<double>> statistics_;
    long count_ = 0;

private:
    virtual void process_next(message::Message msg) = 0;

    virtual void print(std::ostream& os) const = 0;

    friend std::ostream& operator<<(std::ostream& os, const TemporalStatistics& a) {
        a.print(os);
        return os;
    }
};

//-------------------------------------------------------------------------------------------------

class MonthlyStatistics : public TemporalStatistics {

    std::string name_;
    eckit::Date current_;

public:
    MonthlyStatistics(const std::vector<std::string> operations, long fld_sz);

    void process_next(message::Message msg) override;

    void print(std::ostream &os) const override;
};

}
}  // namespace multio

#endif // TEMPORALSTATISTICS_H

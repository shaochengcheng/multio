
#ifndef multio_server_LocalIndices_H
#define multio_server_LocalIndices_H

#include <cstddef>
#include <vector>

namespace multio {
namespace server {

class LocalIndices {
public:
    LocalIndices(std::vector<size_t>&& idx);

    void to_global(const std::vector<double>& local, std::vector<double>& global) const;
    void to_local(const std::vector<double>& global, std::vector<double>& local) const;

private:
    std::vector<size_t> indices_;  // Unstructured
};

}  // namespace server
}  // namespace multio

#endif

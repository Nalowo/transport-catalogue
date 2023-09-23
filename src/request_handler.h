#pragma once
#include <iostream>

#include "transport_catalogue.h"

namespace NS_TransportCatalogue
{
class RequestHandler {
public:

    enum class RequestType {BUS, STOP};
    using Stop_to_Stop_len = std::pair<std::string_view, std::vector<std::pair<std::string_view, unsigned int>>>;

    virtual void ReadInput(std::istream& instream) = 0;
    virtual void PrintRequest(std::ostream& outstream) = 0;

protected:
    
    RequestHandler(TransportCatalogue& db): db_(db) {}
    ~RequestHandler() = default;

    virtual void UploadContent(std::list<domain::Stop>&& stops_to_add, 
                                std::list<Stop_to_Stop_len>&& root_length,
                                std::list<NS_TransportCatalogue::TransportCatalogue::BusInput>&& bus_to_add
                                ) final;

    TransportCatalogue& db_;
};
} // end namespace NS_TransportCatalogue
// */
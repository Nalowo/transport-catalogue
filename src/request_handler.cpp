#include "request_handler.h"

namespace NS_TransportCatalogue
{
    void RequestHandler::UploadContent(std::list<domain::Stop>&& stops_to_add, 
                                std::list<std::pair<std::string_view, std::vector<std::pair<std::string_view, unsigned int>>>>&& root_length,
                                std::list<NS_TransportCatalogue::TransportCatalogue::BusInput>&& bus_to_add
                                )
    {
        db_.AddStop(std::move(stops_to_add), std::move(root_length));
        for (auto& bus : bus_to_add)
        {
            db_.AddBus(std::move(bus));
        }
    }
} // end namespace NS_TransportCatalogue


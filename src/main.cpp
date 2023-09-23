#include <fstream>
#include <iostream>
#include <string_view>
#include <sstream>

#include "request_handler.h"
#include "json_reader.h"
#include "serialization.h"

using namespace std::literals;
using Path = std::filesystem::path;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

void SerializationTC(const std::optional<Path>& file, const NS_TransportCatalogue::TransportCatalogue& db, const NS_TransportCatalogue::Interfaces::JsonReader& reader)
{
    if (file)
    {
        std::ofstream out(*file, std::ios::binary);
        if (!out.good())
        {
            throw std::runtime_error("can't open file - "s + file->string());
        }

        NS_TransportCatalogue::Serealization_Worker::Serealization s_worker(db);
        s_worker.SetMapSettings(&reader.GetRenderSettings());

        s_worker.SetRouterSettings(&reader.GetRouterSettings());
        s_worker.SetGraphBuilder(reader.GetGraphBuilderPtr());
        s_worker.SetRouter(reader.GetRouterPtr());
        s_worker.RunSerealization(out);
    }
}

void MakeBase(std::istream& in)
{
    NS_TransportCatalogue::TransportCatalogue db;
    NS_TransportCatalogue::Interfaces::JsonReader reader{db};
    reader.ReadInput(in);
    reader.RunCreateRouter();

    SerializationTC(reader.GetFilePath(), db, reader);
}

void DeserializationTC(const std::optional<Path>& file, NS_TransportCatalogue::TransportCatalogue& db, NS_TransportCatalogue::Interfaces::JsonReader& reader)
{
    if (file)
    {
        std::ifstream ifs(*file, std::ios::binary);
        if (!ifs.good())
        {
            throw std::runtime_error("can't open file - "s + file->string());
        }

        NS_TransportCatalogue::Serealization_Worker::Deserealization d_worker(db);
        d_worker.RunDeserealization(ifs, reader);
    }
}

void ProcessRequests(std::istream& in, std::ostream& out)
{
    NS_TransportCatalogue::TransportCatalogue db;
    NS_TransportCatalogue::Interfaces::JsonReader reader{db};
    reader.ReadInput(in);

    DeserializationTC(reader.GetFilePath(), db, reader);

    reader.PrintRequest(out);
}

int main(int argc, char* argv[]) {

   if (argc != 2) {
        PrintUsage();
       return EXIT_FAILURE;
    }

    const std::string_view mode(argv[1]);

    if (mode == "make_base"sv) 
    {

        MakeBase(std::cin);

    }
    else if (mode == "process_requests"sv) 
    {

        ProcessRequests(std::cin, std::cout);

    }
    else
    {
        PrintUsage();
        return EXIT_FAILURE;
    }
}

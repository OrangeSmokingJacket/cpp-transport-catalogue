#include "serialization.h"

void Serializator::Save()
{
    std::ofstream ofs(path, std::ios::binary);

    catalogue::WholeMessage whole_message;

    *whole_message.mutable_catalogue() = std::move(handler.SerializeTransportCatalogue());
    *whole_message.mutable_renderer() = std::move(handler.SerializeMapRenderer());
    *whole_message.mutable_router_settings() = std::move(handler.SerializeRouterSettings());

    whole_message.SerializeToOstream(&ofs);
}
void Serializator::Load()
{
    std::ifstream ifs(path, std::ios::binary);
    
    catalogue::WholeMessage whole_message;
	if (!whole_message.ParseFromIstream(&ifs))
	{
        throw std::runtime_error("Couldn't parse input");
	}

    handler.DeserializeTransportCatalogue(whole_message.catalogue());
    handler.DeserializeMapRenderer(whole_message.renderer());
    handler.DeserializeRouterSettings(whole_message.router_settings());
}
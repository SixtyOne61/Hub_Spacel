#include "LocationInformation.h"

FString FLocationInformation::getXml() const
{
    FString xml;

    /*xml = "<location id=\"" + FString::FromInt(m_id) + "\" center=\"" + m_center.ToString() + "\" size=\"" + m_size.ToString() + "\">";
    for (auto const& pair : m_neighbor)
    {
        xml.Append("\n<face type=\"" + FString::FromInt((int)pair.Key) + "\" id=\"" + FString::FromInt(pair.Value->getId()) + "\"/>");
    }
    xml.Append("\n</location>");*/

    return std::move(xml);
}
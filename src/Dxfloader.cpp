#include "Dxfloader.h"
#include <iostream>
#include <Entities/Line.h>
#include <Entities/Circle.h>
#include <Entities/Arc.h>
#include <Entities/Polyline.h>

DxfLoader::DxfLoader() {}

bool DxfLoader::load(const std::string& filename)
{
	DRW_Interface* iface = this;
	dxfRW reader(filename.c_str());
	if (!reader.read(iface, false)) {
		std::cerr << "Failed to load DXF: " << filename << std::endl;
		return false;
	}
	return true;
}

void DxfLoader::addLine(const DRW_Line& data)
{
	std::cout << "Line: (" << data.basePoint.x << "," << data.basePoint.y << ") -> ("
		<< data.secPoint.x << "," << data.secPoint.y << ")\n";

	auto line = std::make_shared<Line>(
		static_cast<float>(data.basePoint.x),
		static_cast<float>(data.basePoint.y),
		static_cast<float>(data.secPoint.x),
		static_cast<float>(data.secPoint.y)
	);

	line->setColor(1.0f, 0.0f, 0.0f);
	_entities.push_back(line);
}

void DxfLoader::addCircle(const DRW_Circle& data) 
{
	auto c = std::make_shared<Circle>(
		static_cast<float>(data.basePoint.x),
		static_cast<float>(data.basePoint.y),
		static_cast<float>(data.radious)
	);
	c->setColor(0.0f,1.0f, 0.0f);
	_entities.push_back(c);
}

void DxfLoader::addLWPolyline(const DRW_LWPolyline& data)
{
	std::cout << "lwply size:" << data.vertlist.size() << std::endl;

	if (data.vertlist.empty())
		return;

	auto polyline = std::make_shared<Polyline>(data);
	polyline->setColor(1.0f, 1.0f, 1.0f); // Example: blue color for polylines
	_entities.push_back(polyline);
}

void DxfLoader::addArc(const DRW_Arc& data) 
{
	auto arc = std::make_shared<Arc>(
		static_cast<float>(data.basePoint.x),
		static_cast<float>(data.basePoint.y),
		static_cast<float>(data.radious),
		static_cast<float>(data.staangle),
		static_cast<float>(data.endangle),
		64
	);

	arc->setColor(1.0f, 0.0f, 0.0f);
	_entities.push_back(arc);
}

void DxfLoader::addPolyline(const DRW_Polyline& data)
{

}
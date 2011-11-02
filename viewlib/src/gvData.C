//===========================================================================
//                                                                           
// File: gvData.C                                                            
//                                                                           
// Created: Fri Apr 20 16:25:33 2001                                         
//                                                                           
// Author: Atgeirr F Rasmussen <atgeirr@sintef.no>
//                                                                           
// Revision: $Id: gvData.C,v 1.4 2008-03-17 15:37:51 sbr Exp $
//                                                                           
// Description:
//                                                                           
//===========================================================================

#include "GoTools/igeslib/IGESconverter.h"
#include "GoTools/viewlib/gvData.h"
#include "GoTools/viewlib/gvObserver.h"
#include "GoTools/viewlib/gvGroupPropertySheet.h"
#include <QObject>
#include <QString>

#include "GoTools/geometry/Utils.h"
#include "GoTools/geometry/Factory.h"
#include "GoTools/geometry/ObjectHeader.h"
#include "GoTools/viewlib/gvCurvePaintable.h"
#include "GoTools/viewlib/gvRectangularSurfacePaintable.h"
#include "GoTools/geometry/ParamCurve.h"
#include "GoTools/tesselator/NoopTesselator.h"
#include "GoTools/viewlib/gvNoopPaintable.h"

#include "GoTools/viewlib/DataHandler.h"


using namespace Go;
using std::vector;
using std::shared_ptr;


//===========================================================================
void gvData::readIges(std::istream& is)
//===========================================================================
{
    //    clear();
    IGESconverter converter;
    try {
	converter.readIGES(is);
    } catch (...) {
	MESSAGE("Something went wrong while reading iges file!");
    }
    vector<shared_ptr<GeomObject> > new_objects = converter.getGoGeom();
    int nmb_new_objs = 0;
    for (size_t i = 0; i < new_objects.size(); ++i) {
	GeomObject* obj = new_objects[i].get();
	if (obj == 0) {
	    MESSAGE("Missing object (NULL pointer)! Continuing anyway.");
	    continue;
	}
	ALWAYS_ERROR_IF(obj->dimension() != 3, "Dimension must be 3.");

	shared_ptr<gvColor> gv_col;
	vector<double> iges_col = converter.getColour((int)i);
	if (iges_col.size() != 0)
	    gv_col = shared_ptr<gvColor>(new gvColor((float)iges_col[0]/100.0f,
						     (float)iges_col[1]/100.0f,
						     (float)iges_col[2]/100.0f,
						     1.0f));
	++nmb_new_objs;
	objects_.push_back(new_objects[i]);
	object_colors_.push_back(gv_col);
    }
    recreateDataStructure(nmb_new_objs);
}

//===========================================================================
void gvData::readSislSrfs(std::istream& is)
//===========================================================================
{
    //    clear();
    IGESconverter converter;
    converter.readsislsrfs(is);
    vector<shared_ptr<GeomObject> > new_objects = converter.getGoGeom();
    int nmb_new_objs = (int)new_objects.size();
    objects_.insert(objects_.end(), new_objects.begin(), new_objects.end());

    for (size_t i = 0; i < new_objects.size(); ++i) {
	GeomObject* obj = new_objects[i].get();
	ALWAYS_ERROR_IF(obj->dimension() != 3, "Dimension must be 3.");

	shared_ptr<gvColor> gv_col;
	vector<double> iges_col = converter.getColour((int)i);
	if (iges_col.size() != 0)
	    gv_col = shared_ptr<gvColor>(new gvColor((float)iges_col[0]/100.0f,
						     (float)iges_col[1]/100.0f,
						     (float)iges_col[2]/100.0f,
						     1.0f));
	object_colors_.push_back(gv_col);
    }
    recreateDataStructure(nmb_new_objs);
}

//===========================================================================
void gvData::readSislCrvs(std::istream& is)
//===========================================================================
{
    //    clear();
    IGESconverter converter;
    converter.readsislcrvs(is);
    vector<shared_ptr<GeomObject> > new_objects = converter.getGoGeom();
    int nmb_new_objs = (int)new_objects.size();
    objects_.insert(objects_.end(), new_objects.begin(), new_objects.end());

    for (size_t i = 0; i < new_objects.size(); ++i) {
	GeomObject* obj = new_objects[i].get();
	ALWAYS_ERROR_IF(obj->dimension() != 3, "Dimension must be 3.");

	shared_ptr<gvColor> gv_col;
	vector<double> iges_col = converter.getColour((int)i);
	if (iges_col.size() != 0)
	    gv_col = shared_ptr<gvColor>(new gvColor((float)iges_col[0]/100.0f,
						     (float)iges_col[1]/100.0f,
						     (float)iges_col[2]/100.0f,
						     1.0f));
	object_colors_.push_back(gv_col);
    }
    recreateDataStructure(nmb_new_objs);
}

//===========================================================================
void gvData::readGo(std::istream& is)
//===========================================================================
{
    //    clear();

    ObjectHeader header;
    int nmb_new_objs = 0;
    while (!is.eof()) {
	try {
	    header.read(is);
	} catch (...) {
	    MESSAGE("Failed reading header, using the geometry read so far.");
	    break;
	}
	//Read(is, header);
	shared_ptr<GeomObject> obj(Factory::createObject(header.classType()));
	try {
	    obj->read(is);
	    //Read(is, *obj);
	    ALWAYS_ERROR_IF(obj->dimension() != 3, "Dimension must be 3.");
	} catch (...) {
	    MESSAGE("Failed reading (Go) object!");
	    obj = shared_ptr<GeomObject>();
	}
	objects_.push_back(obj);
	++nmb_new_objs;
	shared_ptr<gvColor> gv_col;
	if (header.auxdataSize() == 4) {
	    gv_col = shared_ptr<gvColor>(new gvColor((float)header.auxdata(0)/255.0f,
						     (float)header.auxdata(1)/255.0f,
						     (float)header.auxdata(2)/255.0f,
						     (float)header.auxdata(3)/255.0f));
	}
	object_colors_.push_back(gv_col);
	eatwhite(is);
	//SkipComments(is);
    }
    try {
	recreateDataStructure(nmb_new_objs);
    } catch (...) {
	MESSAGE("Failed recreating data structure!");
    }
}


//===========================================================================
void gvData::readGo(const std::vector<shared_ptr<Go::GeomObject> >& new_objects,
		    std::vector<shared_ptr<gvColor> >& new_colors)
//===========================================================================
{
    int nmb_new_objs = (int)new_objects.size();
    if (new_objects.size() != new_colors.size()) {
        new_colors.resize(new_objects.size());
    }

    for (size_t i = 0; i < new_objects.size(); ++i) {
	ALWAYS_ERROR_IF(new_objects[i]->dimension() != 3, "Dimension must be 3.");
    }
    objects_.insert(objects_.end(), new_objects.begin(), new_objects.end());
    object_colors_.insert(object_colors_.end(), new_colors.begin(), new_colors.end());
    recreateDataStructure(nmb_new_objs);
}


//===========================================================================
void gvData::writeSelectedGo(std::ostream& os)
//===========================================================================
{
    os.precision(15);
    for (int i = 0; i < numObjects(); ++i) {
	if (getSelectedStateObject(i)) {
	    vector<int> go_col;
	    if (object_colors_[i].get() != 0) {
		float *rgba = object_colors_[i]->rgba; // 4 elements
		for (int j = 0; j < 4; ++j)
		    go_col.push_back((int)rgba[j]*255);
	    }
	    ObjectHeader header(objects_[i]->instanceType(), MAJOR_VERSION, MINOR_VERSION,
				  go_col);
	    header.write(os);
// 	    objects_[i]->writeStandardHeader(os);
	    os << '\n';
	    objects_[i]->write(os);
	    os << '\n';
	}
    }
    os << std::flush;
}

//===========================================================================
void gvData::writeSelectedIges(std::ostream& os)
//===========================================================================
{
    os.precision(15);
    IGESconverter conv;
    for (int i = 0; i < numObjects(); ++i) {
	if (getSelectedStateObject(i)) {
	  conv.addGeom(objects_[i]);
	}
    }

    conv.writeIGES(os);
}


// //===========================================================================
// void gvData::groupSelected()
// //===========================================================================
// {
//     vector<int> selected;
//     for (int i = 0; i < objects_.size(); ++i)
// 	if (getSelectedStateObject(i))
// 	    selected.push_back(i);

//     if (selected.size() > 0) {
// 	gvGroupPropertySheet group_sheet(selected);
// 	group_sheet.createSheet(0, 0);

// 	// If user clicks OK, group is created and pushed back.
// 	QObject::connect(&group_sheet, SIGNAL(value_changed(vector<int>&, QString)),
// 			 this, SLOT(add_group(vector<int>&, QString)));
//     }
// }

//===========================================================================
void gvData::extractSelectedObjects(std::vector< shared_ptr<Go::GeomObject> >& sel_obj)
//===========================================================================
{
    sel_obj.clear();
    for (int i = 0; i < numObjects(); ++i) {
	if (getSelectedStateObject(i)) {
	   if (objects_[i].get())
	      sel_obj.push_back(objects_[i]);
	}
    }
}



//===========================================================================
void gvData::recreateDataStructure(int nmb_new_objs)
//===========================================================================
{
    //    cout << "gvData::recreateDataStructure... " << flush;

    gvColor blue(0.3, 0.3, 1.0, 1.0);
    gvColor light_blue(0.7, 0.7, 1.0, 1.0);
    gvColor red(1.0, 0.3, 0.3, 1.0);
    gvColor light_red(1.0, 0.7, 0.7, 1.0);

//     painter_.removeAllPaintables();
//     paintables_.clear();
//     tesselators_.clear();
//     property_sheets_.clear();
    int nmb_objs = (int)objects_.size();
    // We start by inserting default values into vectors.
    // Some elements may already have well defined tesselators.
    int i;

    // We make sure that vectors have the right size. Size of vectors should correspond.
    int nmb_old_objs = nmb_objs - nmb_new_objs;
    if (int(tesselators_.size()) != nmb_old_objs) {
	int nmb_to_remove = (int)tesselators_.size() - nmb_old_objs;
	tesselators_.resize(nmb_old_objs);
	paintables_.resize(nmb_old_objs);
	property_sheets_.resize(nmb_old_objs);
	for (i = 0; i < nmb_to_remove; ++i) {
	    painter_.removeLastPaintable();
	}
    }

    for (i = nmb_old_objs; i < nmb_old_objs + nmb_new_objs; ++i) {

	//	cout << "Iteration " << i << endl;
	if (objects_[i].get()==NULL) {
	    MESSAGE("Object missing!");
	    continue;
	}
	gvColor col;
	// @@ Hack to get red curves by default, other things are blue...
//  	if (dynamic_cast<ParamCurve*>(objects_[i].get())) {
//  	    col = red;
//  	}
	if (object_colors_[i].get() != 0) {
	    col = *object_colors_[i];
	} else {
 	    col = blue;
// 	    col = gvColor(0.3, 0.3, 1.0, 1.0);
	}
	datahandler_->clear();
	try {
	    datahandler_->create(objects_[i], col, i);
	    datahandler_->tesselator()->tesselate();
	    tesselators_.push_back(datahandler_->tesselator());
	    paintables_.push_back(datahandler_->paintable());
	    property_sheets_.push_back(datahandler_->propertySheet());
	    painter_.addPaintable(datahandler_->paintable());
	} catch (...) {
	    MESSAGE("Failed creating some object (index: " << i << ")!");
	    tesselators_.push_back(shared_ptr<Tesselator>(new NoopTesselator()));
	    paintables_.push_back(shared_ptr<gvPaintable>(new gvNoopPaintable(col, i)));
	    property_sheets_.push_back(shared_ptr<gvPropertySheet>());
	    painter_.addPaintable(shared_ptr<gvPaintable>());
	}
    }
    try {
	computeBox();
	updateObservers();
    } catch (...) {
	MESSAGE("Failed ...");
    }
    //    cout << " finished" << endl;
}


//===========================================================================
void gvData::computeBox()
//===========================================================================
{
   int i;
   for (i = 0; i < numObjects(); ++i) {
      if (object(i).get())
      {
	 box_ = object(i)->boundingBox();
	 break;
      }
   }
   for (; i < numObjects(); ++i) {
      if (object(i).get())
      {
	  try {
	      box_.addUnionWith(object(i)->boundingBox());
	  } catch (...) {
	      MESSAGE("Failed adding union with next box!");
	  }
      }
   }
}

//===========================================================================
Go::BoundingBox gvData::boundingBox(const std::vector<int>& objs) const
//===========================================================================
{
   Go::BoundingBox box;
   for (size_t i = 0; i < objs.size(); ++i)  {
       if (objs.size()>0 && numObjects()>objs[i] && object(objs[i]).get()) {
	   box=object(objs[i])->boundingBox();
	   break;
       }
       for (; i < objs.size(); ++i) {
	   if (objs.size()>0 && numObjects()>objs[i] && object(objs[i]).get()) {
	       box.addUnionWith(object(objs[i])->boundingBox());
	   }
       }
   }
   return box;
}


//===========================================================================
void gvData::updateObservers()
//===========================================================================
{
    if (!updates_enabled_) return;
    std::set<gvObserver*>::iterator it;
    for (it = observers_.begin(); it != observers_.end(); ++it) {
	(*it)->observedChanged();
    }
}

//===========================================================================
void gvData::setSelectedStateObject(int id, bool state)
//===========================================================================
{
   if (id < int(objects_.size()) && objects_[id].get())
   {
      bool oldstate = paintables_[id]->selected();
      if (state != oldstate) {
	 paintables_[id]->setSelected(state);
	 updateObservers();
      }
   }
}


//===========================================================================
void gvData::setVisibleStateObject(int id, bool state)
//===========================================================================
{
   if (id < int(objects_.size()) && objects_[id].get())
   {
      bool oldstate = paintables_[id]->visible();
      if (state != oldstate) {
	 paintables_[id]->setVisible(state);
	 updateObservers();
      }
   }
}

//===========================================================================
bool gvData::getSelectedStateObject(int id)
//===========================================================================
{
   if (paintables_[id].get())
    return paintables_[id]->selected();
   else
      return false;
}


//===========================================================================
bool gvData::getVisibleStateObject(int id)
//===========================================================================
{
   if (paintables_[id].get())
      return paintables_[id]->visible();
   else
      return false;
}

//===========================================================================
void gvData::clear()
//===========================================================================
{
    objects_.clear();
    object_colors_.clear();
    // box_ is unchanged
    object_groups_.clear();

    property_sheets_.clear();
    tesselators_.clear();
    paintables_.clear();

    painter_.removeAllPaintables();
}

//===========================================================================
void gvData::deleteObj(int obj)
//===========================================================================
{
   if (obj == int(objects_.size())-1)
      clearLast();
   else
   {
      objects_[obj].reset();
      object_colors_[obj].reset();
      property_sheets_[obj].reset();
      tesselators_[obj].reset();
      paintables_[obj].reset();
      painter_.removePaintable(obj);
   }
   updateObservers();
}

//===========================================================================
void gvData::clearLast()
//===========================================================================
{
    objects_.erase(objects_.end() - 1);
    object_colors_.erase(object_colors_.end() - 1);
    // box_ is unchanged
    // @@sbr Hmm, maybe we should make sure that object is not part of a group?

    property_sheets_.erase(property_sheets_.end() - 1);
    tesselators_.erase(tesselators_.end() - 1);
    paintables_.erase(paintables_.end() - 1);

    painter_.removeLastPaintable();
    if (objects_.size()>0 && objects_[objects_.size()-1].get()==NULL)
      clearLast();
}
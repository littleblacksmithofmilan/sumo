/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2001-2019 German Aerospace Center (DLR) and others.
// This program and the accompanying materials
// are made available under the terms of the Eclipse Public License v2.0
// which accompanies this distribution, and is available at
// http://www.eclipse.org/legal/epl-v20.html
// SPDX-License-Identifier: EPL-2.0
/****************************************************************************/
/// @file    GNEVehicleTypeDialog.cpp
/// @author  Pablo Alvarez Lopez
/// @date    Jan 2019
/// @version $Id$
///
// Dialog for edit vehicleTypes
/****************************************************************************/

// ===========================================================================
// included modules
// ===========================================================================
#include <config.h>

#include <iostream>
#include <utils/common/MsgHandler.h>
#include <utils/gui/windows/GUIAppEnum.h>
#include <utils/gui/images/GUIIconSubSys.h>
#include <utils/gui/div/GUIDesigns.h>
#include <netedit/changes/GNEChange_DemandElement.h>
#include <netedit/additionals/GNECalibrator.h>
#include <netedit/netelements/GNEEdge.h>
#include <netedit/netelements/GNELane.h>
#include <netedit/GNEViewNet.h>
#include <netedit/GNENet.h>
#include <netedit/demandelements/GNEVehicleType.h>
#include <netedit/GNEUndoList.h>

#include "GNEVehicleTypeDialog.h"


// ===========================================================================
// FOX callback mapping
// ===========================================================================

FXDEFMAP(GNEVehicleTypeDialog) GNEVehicleTypeDialogMap[] = {
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_CALIBRATORDIALOG_SET_VARIABLE,  GNEVehicleTypeDialog::onCmdSetVariable),
};

// Object implementation
FXIMPLEMENT(GNEVehicleTypeDialog, GNEDemandElementDialog, GNEVehicleTypeDialogMap, ARRAYNUMBER(GNEVehicleTypeDialogMap))

// ===========================================================================
// member method definitions
// ===========================================================================

GNEVehicleTypeDialog::GNEVehicleTypeDialog(GNEDemandElement* editedVehicleType, bool updatingElement) :
    GNEDemandElementDialog(editedVehicleType, updatingElement, 1000, 350),
    myVehicleTypeValid(true),
    myInvalidAttr(SUMO_ATTR_NOTHING) {

    // change default header
    changeDemandElementDialogHeader(updatingElement ? "Edit " + myEditedDemandElement->getTagStr() + " of " : "Create " + myEditedDemandElement->getTagStr());
    
    // Create auxiliar frames for values
    FXHorizontalFrame* columns = new FXHorizontalFrame(myContentFrame, GUIDesignAuxiliarHorizontalFrame);

    myVTypeCommonAtributes = new VTypeCommonAtributes(this, columns);

    myCarFollowingModelParameters = new CarFollowingModelParameters(this, columns);

    // update fields
    updateVehicleTypeValues();

    // start a undo list for editing local to this additional
    initChanges();

    // add element if we aren't updating an existent element
    if (myUpdatingElement == false) {
        myEditedDemandElement->getViewNet()->getUndoList()->add(new GNEChange_DemandElement(myEditedDemandElement, true), true);
    }

    // open as modal dialog
    openAsModalDialog();
}


GNEVehicleTypeDialog::~GNEVehicleTypeDialog() {}


long
GNEVehicleTypeDialog::onCmdAccept(FXObject*, FXSelector, void*) {
    if (myVehicleTypeValid == false) {
        // write warning if netedit is running in testing mode
        WRITE_DEBUG("Opening FXMessageBox of type 'warning'");
        std::string operation1 = myUpdatingElement ? ("updating") : ("creating");
        std::string operation2 = myUpdatingElement ? ("updated") : ("created");
        std::string tagString = myEditedDemandElement->getTagStr();
        // open warning dialogBox
        FXMessageBox::warning(getApp(), MBOX_OK,
                              ("Error " + operation1 + " " + tagString).c_str(), "%s",
                              (tagString + " cannot be " + operation2 +
                               " because parameter " + toString(myInvalidAttr) +
                               " is invalid.").c_str());
        // write warning if netedit is running in testing mode
        WRITE_DEBUG("Closed FXMessageBox of type 'warning' with 'OK'");
        return 0;
    } else {
        // accept changes before closing dialog
        acceptChanges();
        // stop dialgo sucesfully
        getApp()->stopModal(this, TRUE);
        return 1;
    }
}


long
GNEVehicleTypeDialog::onCmdCancel(FXObject*, FXSelector, void*) {
    // cancel changes
    cancelChanges();
    // Stop Modal
    getApp()->stopModal(this, FALSE);
    return 1;
}


long
GNEVehicleTypeDialog::onCmdReset(FXObject*, FXSelector, void*) {
    // reset changes
    resetChanges();
    // update fields
    updateVehicleTypeValues();
    return 1;
}


long
GNEVehicleTypeDialog::onCmdSetVariable(FXObject*, FXSelector, void*) {
    // At start we assumed, that all values are valid
    myVehicleTypeValid = true;
    myInvalidAttr = SUMO_ATTR_NOTHING;
    // set car following model rows
    myCarFollowingModelParameters->setVariable();
    myVTypeCommonAtributes->setVariable();
    return 1;
}


void
GNEVehicleTypeDialog::updateVehicleTypeValues() {
    // update values of Vehicle Type common attributes
    myVTypeCommonAtributes->updateValues();
    // update values of Car Following Model Parameters
    myCarFollowingModelParameters->updateValues();
}

// ---------------------------------------------------------------------------
// GNEVehicleTypeDialog::VClassRow - methods
// ---------------------------------------------------------------------------

GNEVehicleTypeDialog::VClassRow::VClassRow(GNEVehicleTypeDialog* vehicleTypeDialog, FXVerticalFrame* column) :
    FXHorizontalFrame(vehicleTypeDialog),
    myVehicleTypeDialog(vehicleTypeDialog) {
    // create two auxiliar frames
    FXHorizontalFrame* horizontalFrame = new FXHorizontalFrame(column, GUIDesignAuxiliarHorizontalFrame);
    FXVerticalFrame* verticalFrame = new FXVerticalFrame(horizontalFrame, GUIDesignAuxiliarVerticalFrame);
    // create FXComboBox for VClass
    new FXLabel(verticalFrame, toString(SUMO_ATTR_VCLASS).c_str(), nullptr, GUIDesignLabelAttribute150);
    myComboBoxVClass = new FXComboBox(verticalFrame, GUIDesignComboBoxNCol, vehicleTypeDialog, MID_GNE_CALIBRATORDIALOG_SET_VARIABLE, GUIDesignComboBox);
    myComboBoxVClassLabelImage = new FXLabel(horizontalFrame, "", nullptr, GUIDesignLabelIconExtendedx46Ticked);
    myComboBoxVClassLabelImage->setBackColor(FXRGBA(255, 255, 255, 255));
    // fill combo Box with all VClass
    std::vector<std::string> VClassStrings = SumoVehicleClassStrings.getStrings();
    for (auto i : VClassStrings) {
        if (i != SumoVehicleClassStrings.getString(SVC_IGNORING)) {
            myComboBoxVClass->appendItem(i.c_str());
        }
    }
    // only show 10 VClasses
    myComboBoxVClass->setNumVisible(10);
}


void 
GNEVehicleTypeDialog::VClassRow::setVariable() {
    // set color of myComboBoxVClass, depending if current value is valid or not
    myComboBoxVClass->setTextColor(FXRGB(0, 0, 0));
    if (myVehicleTypeDialog->myEditedDemandElement->isValid(SUMO_ATTR_VCLASS, myComboBoxVClass->getText().text())) {
        myComboBoxVClass->setTextColor(FXRGB(0, 0, 0));
        myVehicleTypeDialog->myEditedDemandElement->setAttribute(SUMO_ATTR_VCLASS, myComboBoxVClass->getText().text(), myVehicleTypeDialog->myEditedDemandElement->getViewNet()->getUndoList());
        setVClassLabelImage();
    } else {
        myComboBoxVClass->setTextColor(FXRGB(255, 0, 0));
        myVehicleTypeDialog->myVehicleTypeValid = false;
        myVehicleTypeDialog->myInvalidAttr = SUMO_ATTR_VCLASS;
    }
}


void 
GNEVehicleTypeDialog::VClassRow::updateValues() {
    myComboBoxVClass->setText(myVehicleTypeDialog->myEditedDemandElement->getAttribute(SUMO_ATTR_VCLASS).c_str());
    setVClassLabelImage();
}


void
GNEVehicleTypeDialog::VClassRow::setVClassLabelImage() {
    // set Icon in label depending of current VClass
    switch (getVehicleClassID(myVehicleTypeDialog->myEditedDemandElement->getAttribute(SUMO_ATTR_VCLASS))) {
        case SVC_PRIVATE:
            myComboBoxVClassLabelImage->setIcon(GUIIconSubSys::getIcon(ICON_VCLASS_PRIVATE));
            break;
        case SVC_EMERGENCY:
            myComboBoxVClassLabelImage->setIcon(GUIIconSubSys::getIcon(ICON_VCLASS_EMERGENCY));
            break;
        case SVC_AUTHORITY:
            myComboBoxVClassLabelImage->setIcon(GUIIconSubSys::getIcon(ICON_VCLASS_AUTHORITY));
            break;
        case SVC_ARMY:
            myComboBoxVClassLabelImage->setIcon(GUIIconSubSys::getIcon(ICON_VCLASS_ARMY));
            break;
        case SVC_VIP:
            myComboBoxVClassLabelImage->setIcon(GUIIconSubSys::getIcon(ICON_VCLASS_VIP));
            break;
        case SVC_PASSENGER:
            myComboBoxVClassLabelImage->setIcon(GUIIconSubSys::getIcon(ICON_VCLASS_PASSENGER));
            break;
        case SVC_HOV:
            myComboBoxVClassLabelImage->setIcon(GUIIconSubSys::getIcon(ICON_VCLASS_HOV));
            break;
        case SVC_TAXI:
            myComboBoxVClassLabelImage->setIcon(GUIIconSubSys::getIcon(ICON_VCLASS_TAXI));
            break;
        case SVC_BUS:
            myComboBoxVClassLabelImage->setIcon(GUIIconSubSys::getIcon(ICON_VCLASS_BUS));
            break;
        case SVC_COACH:
            myComboBoxVClassLabelImage->setIcon(GUIIconSubSys::getIcon(ICON_VCLASS_COACH));
            break;
        case SVC_DELIVERY:
            myComboBoxVClassLabelImage->setIcon(GUIIconSubSys::getIcon(ICON_VCLASS_DELIVERY));
            break;
        case SVC_TRUCK:
            myComboBoxVClassLabelImage->setIcon(GUIIconSubSys::getIcon(ICON_VCLASS_TRUCK));
            break;
        case SVC_TRAILER:
            myComboBoxVClassLabelImage->setIcon(GUIIconSubSys::getIcon(ICON_VCLASS_TRAILER));
            break;
        case SVC_TRAM:
            myComboBoxVClassLabelImage->setIcon(GUIIconSubSys::getIcon(ICON_VCLASS_TRAM));
            break;
        case SVC_RAIL_URBAN:
            myComboBoxVClassLabelImage->setIcon(GUIIconSubSys::getIcon(ICON_VCLASS_RAIL_URBAN));
            break;
        case SVC_RAIL:
            myComboBoxVClassLabelImage->setIcon(GUIIconSubSys::getIcon(ICON_VCLASS_RAIL));
            break;
        case SVC_RAIL_ELECTRIC:
            myComboBoxVClassLabelImage->setIcon(GUIIconSubSys::getIcon(ICON_VCLASS_RAIL_ELECTRIC));
            break;
        case SVC_MOTORCYCLE:
            myComboBoxVClassLabelImage->setIcon(GUIIconSubSys::getIcon(ICON_VCLASS_MOTORCYCLE));
            break;
        case SVC_MOPED:
            myComboBoxVClassLabelImage->setIcon(GUIIconSubSys::getIcon(ICON_VCLASS_MOPED));
            break;
        case SVC_BICYCLE:
            myComboBoxVClassLabelImage->setIcon(GUIIconSubSys::getIcon(ICON_VCLASS_BICYCLE));
            break;
        case SVC_PEDESTRIAN:
            myComboBoxVClassLabelImage->setIcon(GUIIconSubSys::getIcon(ICON_VCLASS_PEDESTRIAN));
            break;
        case SVC_E_VEHICLE:
            myComboBoxVClassLabelImage->setIcon(GUIIconSubSys::getIcon(ICON_VCLASS_EVEHICLE));
            break;
        case SVC_SHIP:
            myComboBoxVClassLabelImage->setIcon(GUIIconSubSys::getIcon(ICON_VCLASS_SHIP));
            break;
        case SVC_CUSTOM1:
            myComboBoxVClassLabelImage->setIcon(GUIIconSubSys::getIcon(ICON_VCLASS_CUSTOM1));
            break;
        case SVC_CUSTOM2:
            myComboBoxVClassLabelImage->setIcon(GUIIconSubSys::getIcon(ICON_VCLASS_CUSTOM2));
            break;
        default:
            myComboBoxVClassLabelImage->setIcon(GUIIconSubSys::getIcon(ICON_VCLASS_IGNORING));
            break;
    }
}

// ---------------------------------------------------------------------------
// GNEVehicleTypeDialog::VShapeRow - methods
// ---------------------------------------------------------------------------

GNEVehicleTypeDialog::VShapeRow::VShapeRow(GNEVehicleTypeDialog* vehicleTypeDialog, FXVerticalFrame* column) :
    FXHorizontalFrame(vehicleTypeDialog),
    myVehicleTypeDialog(vehicleTypeDialog) {
    // create two auxiliar frames
    FXHorizontalFrame* horizontalFrame = new FXHorizontalFrame(column, GUIDesignAuxiliarHorizontalFrame);
    FXVerticalFrame* verticalFrame = new FXVerticalFrame(horizontalFrame, GUIDesignAuxiliarVerticalFrame);
    // create combo bof for vehicle shapes
    new FXLabel(verticalFrame, toString(SUMO_ATTR_GUISHAPE).c_str(), nullptr, GUIDesignLabelAttribute150);
    myComboBoxShape = new FXComboBox(verticalFrame, GUIDesignComboBoxNCol, vehicleTypeDialog, MID_GNE_CALIBRATORDIALOG_SET_VARIABLE, GUIDesignComboBox);
    myComboBoxShapeLabelImage = new FXLabel(horizontalFrame, "", nullptr, GUIDesignLabelIconExtendedx46Ticked);
    myComboBoxShapeLabelImage->setBackColor(FXRGBA(255, 255, 255, 255));
    // fill combo Box with all vehicle shapes
    std::vector<std::string> VShapeStrings = SumoVehicleShapeStrings.getStrings();
    for (auto i : VShapeStrings) {
        if (i != SumoVehicleShapeStrings.getString(SVS_UNKNOWN)) {
            myComboBoxShape->appendItem(i.c_str());
        }
    }
    // only show 10 Shapes
    myComboBoxShape->setNumVisible(10);
}


void 
GNEVehicleTypeDialog::VShapeRow::setVariable() {
    // set color of myComboBoxShape, depending if current value is valid or not
    if (myVehicleTypeDialog->myEditedDemandElement->isValid(SUMO_ATTR_GUISHAPE, myComboBoxShape->getText().text())) {
        myComboBoxShape->setTextColor(FXRGB(0, 0, 0));
        myVehicleTypeDialog->myEditedDemandElement->setAttribute(SUMO_ATTR_GUISHAPE, myComboBoxShape->getText().text(), myVehicleTypeDialog->myEditedDemandElement->getViewNet()->getUndoList());
        setVShapeLabelImage();
    } else {
        myComboBoxShape->setTextColor(FXRGB(255, 0, 0));
        myVehicleTypeDialog->myVehicleTypeValid = false;
        myVehicleTypeDialog->myInvalidAttr = SUMO_ATTR_GUISHAPE;
    }
}


void 
GNEVehicleTypeDialog::VShapeRow::updateValues() {
    myComboBoxShape->setText(myVehicleTypeDialog->myEditedDemandElement->getAttribute(SUMO_ATTR_GUISHAPE).c_str());
    setVShapeLabelImage();
}


void
GNEVehicleTypeDialog::VShapeRow::setVShapeLabelImage() {
    // set Icon in label depending of current VClass
    switch (getVehicleShapeID(myVehicleTypeDialog->myEditedDemandElement->getAttribute(SUMO_ATTR_GUISHAPE))) {
        case SVS_UNKNOWN:
            myComboBoxShapeLabelImage->setIcon(GUIIconSubSys::getIcon(ICON_VSHAPE_UNKNOWN));
            break;
        case SVS_PEDESTRIAN:
            myComboBoxShapeLabelImage->setIcon(GUIIconSubSys::getIcon(ICON_VSHAPE_PEDESTRIAN));
            break;
        case SVS_BICYCLE:
            myComboBoxShapeLabelImage->setIcon(GUIIconSubSys::getIcon(ICON_VSHAPE_BICYCLE));
            break;
        case SVS_MOPED:
            myComboBoxShapeLabelImage->setIcon(GUIIconSubSys::getIcon(ICON_VSHAPE_MOPED));
            break;
        case SVS_MOTORCYCLE:
            myComboBoxShapeLabelImage->setIcon(GUIIconSubSys::getIcon(ICON_VSHAPE_MOTORCYCLE));
            break;
        case SVS_PASSENGER:
            myComboBoxShapeLabelImage->setIcon(GUIIconSubSys::getIcon(ICON_VSHAPE_PASSENGER));
            break;
        case SVS_PASSENGER_SEDAN:
            myComboBoxShapeLabelImage->setIcon(GUIIconSubSys::getIcon(ICON_VSHAPE_PASSENGER_SEDAN));
            break;
        case SVS_PASSENGER_HATCHBACK:
            myComboBoxShapeLabelImage->setIcon(GUIIconSubSys::getIcon(ICON_VSHAPE_PASSENGER_HATCHBACK));
            break;
        case SVS_PASSENGER_WAGON:
            myComboBoxShapeLabelImage->setIcon(GUIIconSubSys::getIcon(ICON_VSHAPE_PASSENGER_WAGON));
            break;
        case SVS_PASSENGER_VAN:
            myComboBoxShapeLabelImage->setIcon(GUIIconSubSys::getIcon(ICON_VSHAPE_PASSENGER_VAN));
            break;
        case SVS_DELIVERY:
            myComboBoxShapeLabelImage->setIcon(GUIIconSubSys::getIcon(ICON_VSHAPE_DELIVERY));
            break;
        case SVS_TRUCK:
            myComboBoxShapeLabelImage->setIcon(GUIIconSubSys::getIcon(ICON_VSHAPE_TRUCK));
            break;
        case SVS_TRUCK_SEMITRAILER:
            myComboBoxShapeLabelImage->setIcon(GUIIconSubSys::getIcon(ICON_VSHAPE_TRUCK_SEMITRAILER));
            break;
        case SVS_TRUCK_1TRAILER:
            myComboBoxShapeLabelImage->setIcon(GUIIconSubSys::getIcon(ICON_VSHAPE_TRUCK_1TRAILER));
            break;
        case SVS_BUS:
            myComboBoxShapeLabelImage->setIcon(GUIIconSubSys::getIcon(ICON_VSHAPE_BUS));
            break;
        case SVS_BUS_COACH:
            myComboBoxShapeLabelImage->setIcon(GUIIconSubSys::getIcon(ICON_VSHAPE_BUS_COACH));
            break;
        case SVS_BUS_FLEXIBLE:
            myComboBoxShapeLabelImage->setIcon(GUIIconSubSys::getIcon(ICON_VSHAPE_BUS_FLEXIBLE));
            break;
        case SVS_BUS_TROLLEY:
            myComboBoxShapeLabelImage->setIcon(GUIIconSubSys::getIcon(ICON_VSHAPE_BUS_TROLLEY));
            break;
        case SVS_RAIL:
            myComboBoxShapeLabelImage->setIcon(GUIIconSubSys::getIcon(ICON_VSHAPE_RAIL));
            break;
        case SVS_RAIL_CAR:
            myComboBoxShapeLabelImage->setIcon(GUIIconSubSys::getIcon(ICON_VSHAPE_RAIL_CAR));
            break;
        case SVS_RAIL_CARGO:
            myComboBoxShapeLabelImage->setIcon(GUIIconSubSys::getIcon(ICON_VSHAPE_RAIL_CARGO));
            break;
        case SVS_E_VEHICLE:
            myComboBoxShapeLabelImage->setIcon(GUIIconSubSys::getIcon(ICON_VSHAPE_E_VEHICLE));
            break;
        case SVS_ANT:
            myComboBoxShapeLabelImage->setIcon(GUIIconSubSys::getIcon(ICON_VSHAPE_ANT));
            break;
        case SVS_SHIP:
            myComboBoxShapeLabelImage->setIcon(GUIIconSubSys::getIcon(ICON_VSHAPE_SHIP));
            break;
        case SVS_EMERGENCY:
        case SVS_FIREBRIGADE:
            myComboBoxShapeLabelImage->setIcon(GUIIconSubSys::getIcon(ICON_VSHAPE_EMERGENCY));
            break;
        case SVS_POLICE:
            myComboBoxShapeLabelImage->setIcon(GUIIconSubSys::getIcon(ICON_VSHAPE_POLICE));
            break;
        case SVS_RICKSHAW:
            myComboBoxShapeLabelImage->setIcon(GUIIconSubSys::getIcon(ICON_VSHAPE_RICKSHAW));
            break;
        default:
            myComboBoxShapeLabelImage->setIcon(GUIIconSubSys::getIcon(ICON_VCLASS_IGNORING));
            break;
    }
}

// ---------------------------------------------------------------------------
// GNEVehicleTypeDialog::VTypeCommonAtributes - methods
// ---------------------------------------------------------------------------

GNEVehicleTypeDialog::VTypeCommonAtributes::VTypeCommonAtributes(GNEVehicleTypeDialog* vehicleTypeDialog, FXHorizontalFrame* column) :
    FXGroupBox(column, "Vehicle Type attributes", GUIDesignGroupBoxFrame),
    myVehicleTypeDialog(vehicleTypeDialog) {

    FXHorizontalFrame* columnsCommonVTypes = new FXHorizontalFrame(this, GUIDesignAuxiliarHorizontalFrame);

    buildCommonAttributesA(new FXVerticalFrame(columnsCommonVTypes, GUIDesignAuxiliarFrame));

    buildCommonAttributesB(new FXVerticalFrame(columnsCommonVTypes, GUIDesignAuxiliarFrame));
}


void 
GNEVehicleTypeDialog::VTypeCommonAtributes::buildCommonAttributesA(FXVerticalFrame* column) {
    // 01 Create VClassRow
    myVClassRow = new VClassRow(myVehicleTypeDialog, column);

    // 02 create FXTextField and Label for vehicleTypeID
    myTextFieldVehicleTypeID = myVehicleTypeDialog->buildRowFloat(column, SUMO_ATTR_ID);

    // 03 create FXTextField and Label for Color
    myTextFieldColor = myVehicleTypeDialog->buildRowString(column, SUMO_ATTR_COLOR);

    // 04 create FXTextField and Label for Length
    myTextFieldLength = myVehicleTypeDialog->buildRowFloat(column, SUMO_ATTR_LENGTH);

    // 05 create FXTextField and Label for MinGap
    myTextFieldMinGap = myVehicleTypeDialog->buildRowFloat(column, SUMO_ATTR_MINGAP);

    // 06 create FXTextField and Label for MaxSpeed
    myTextFieldMaxSpeed = myVehicleTypeDialog->buildRowFloat(column, SUMO_ATTR_MAXSPEED);

    // 07 create FXTextField and Label for SpeedFactor
    myTextFieldSpeedFactor = myVehicleTypeDialog->buildRowFloat(column, SUMO_ATTR_SPEEDFACTOR);

    // 08 create FXTextField and Label for SpeedDev
    myTextFieldSpeedDev = myVehicleTypeDialog->buildRowFloat(column, SUMO_ATTR_SPEEDDEV);

    // 09 create FXTextField and Label for EmissionClass
    myTextFieldEmissionClass = myVehicleTypeDialog->buildRowFloat(column, SUMO_ATTR_EMISSIONCLASS);

    // 10 create FXTextField and Label for Width
    myTextFieldWidth = myVehicleTypeDialog->buildRowFloat(column, SUMO_ATTR_WIDTH);

    // 11 create FXTextField and Label for Filename
    myTextFieldFilename = myVehicleTypeDialog->buildRowString(column, SUMO_ATTR_IMGFILE);
}


void 
GNEVehicleTypeDialog::VTypeCommonAtributes::buildCommonAttributesB(FXVerticalFrame* column) {
    // 01 Create VShapeRow
    myVShapeRow = new VShapeRow(myVehicleTypeDialog, column);

    // 02 create FXTextField and Label for Impatience
    myTextFieldImpatience = myVehicleTypeDialog->buildRowFloat(column, SUMO_ATTR_IMPATIENCE);

    // 03 create FXTextField and Label for LaneChangeModel
    myTextFieldLaneChangeModel = myVehicleTypeDialog->buildRowFloat(column, SUMO_ATTR_LANE_CHANGE_MODEL);

    // 04 create FXTextField and Label for PersonCapacity
    myTextFieldPersonCapacity = myVehicleTypeDialog->buildRowInt(column, SUMO_ATTR_PERSON_CAPACITY);

    // 05 create FXTextField and Label for ContainerCapacity
    myTextFieldContainerCapacity = myVehicleTypeDialog->buildRowInt(column, SUMO_ATTR_CONTAINER_CAPACITY);

    // 06 create FXTextField and Label for BoardingDuration
    myTextFieldBoardingDuration = myVehicleTypeDialog->buildRowFloat(column, SUMO_ATTR_BOARDING_DURATION);

    // 07 create FXTextField and Label for LoadingDuration
    myTextFieldLoadingDuration = myVehicleTypeDialog->buildRowFloat(column, SUMO_ATTR_LOADING_DURATION);

    // 08 create FXTextField and Label for LatAlignment
    myTextFieldLatAlignment = myVehicleTypeDialog->buildRowString(column, SUMO_ATTR_LATALIGNMENT);

    // 09 create FXTextField and Label for MinGapLat
    myTextFieldMinGapLat = myVehicleTypeDialog->buildRowFloat(column, SUMO_ATTR_MINGAP_LAT);

    // 10 create FXTextField and Label for MaxSpeedLat
    myTextFieldMaxSpeedLat = myVehicleTypeDialog->buildRowFloat(column, SUMO_ATTR_MAXSPEED_LAT);

    // 11 create FXTextField and Label for ActionStepLenght
    myTextFieldActionStepLenght = myVehicleTypeDialog->buildRowFloat(column, SUMO_ATTR_ACTIONSTEPLENGTH);
}


void 
GNEVehicleTypeDialog::VTypeCommonAtributes::setVariable() {
    // set variables of special rows VType and VShape
    myVClassRow->setVariable();
    myVShapeRow->setVariable();
    // set color of myTextFieldVehicleTypeID, depending if current value is valid or not
    if (myVehicleTypeDialog->myEditedDemandElement->isValid(SUMO_ATTR_ID, myTextFieldVehicleTypeID->getText().text())) {
        myTextFieldVehicleTypeID->setTextColor(FXRGB(0, 0, 0));
        myVehicleTypeDialog->myEditedDemandElement->setAttribute(SUMO_ATTR_ID, myTextFieldVehicleTypeID->getText().text(), myVehicleTypeDialog->myEditedDemandElement->getViewNet()->getUndoList());
    } else if (myVehicleTypeDialog->myEditedDemandElement->getAttribute(SUMO_ATTR_ID) == myTextFieldVehicleTypeID->getText().text()) {
        myTextFieldVehicleTypeID->setTextColor(FXRGB(0, 0, 0));
        myVehicleTypeDialog->myEditedDemandElement->setAttribute(SUMO_ATTR_ID, myTextFieldVehicleTypeID->getText().text(), myVehicleTypeDialog->myEditedDemandElement->getViewNet()->getUndoList());
    } else {
        myTextFieldVehicleTypeID->setTextColor(FXRGB(255, 0, 0));
        myVehicleTypeDialog->myVehicleTypeValid = false;
        myVehicleTypeDialog->myInvalidAttr = SUMO_ATTR_ID;
    }
    // set color of myTextFieldLength, depending if current value is valid or not
    if (myVehicleTypeDialog->myEditedDemandElement->isValid(SUMO_ATTR_LENGTH, myTextFieldLength->getText().text())) {
        myTextFieldLength->setTextColor(FXRGB(0, 0, 0));
        myVehicleTypeDialog->myEditedDemandElement->setAttribute(SUMO_ATTR_LENGTH, myTextFieldLength->getText().text(), myVehicleTypeDialog->myEditedDemandElement->getViewNet()->getUndoList());
    } else {
        myTextFieldLength->setTextColor(FXRGB(255, 0, 0));
        myVehicleTypeDialog->myVehicleTypeValid = false;
        myVehicleTypeDialog->myInvalidAttr = SUMO_ATTR_LENGTH;
    }
    // set color of myTextFieldMinGap, depending if current value is valid or not
    if (myVehicleTypeDialog->myEditedDemandElement->isValid(SUMO_ATTR_MINGAP, myTextFieldMinGap->getText().text())) {
        myTextFieldMinGap->setTextColor(FXRGB(0, 0, 0));
        myVehicleTypeDialog->myEditedDemandElement->setAttribute(SUMO_ATTR_MINGAP, myTextFieldMinGap->getText().text(), myVehicleTypeDialog->myEditedDemandElement->getViewNet()->getUndoList());
    } else {
        myTextFieldMinGap->setTextColor(FXRGB(255, 0, 0));
        myVehicleTypeDialog->myVehicleTypeValid = false;
        myVehicleTypeDialog->myInvalidAttr = SUMO_ATTR_MINGAP;
    }
    // set color of myTextFieldMaxSpeed, depending if current value is valid or not
    if (myVehicleTypeDialog->myEditedDemandElement->isValid(SUMO_ATTR_MAXSPEED, myTextFieldMaxSpeed->getText().text())) {
        myTextFieldMaxSpeed->setTextColor(FXRGB(0, 0, 0));
        myVehicleTypeDialog->myEditedDemandElement->setAttribute(SUMO_ATTR_MAXSPEED, myTextFieldMaxSpeed->getText().text(), myVehicleTypeDialog->myEditedDemandElement->getViewNet()->getUndoList());
    } else {
        myTextFieldMaxSpeed->setTextColor(FXRGB(255, 0, 0));
        myVehicleTypeDialog->myVehicleTypeValid = false;
        myVehicleTypeDialog->myInvalidAttr = SUMO_ATTR_MAXSPEED;
    }
    // set color of myTextFieldSpeedFactor, depending if current value is valid or not
    if (myVehicleTypeDialog->myEditedDemandElement->isValid(SUMO_ATTR_SPEEDFACTOR, myTextFieldSpeedFactor->getText().text())) {
        myTextFieldSpeedFactor->setTextColor(FXRGB(0, 0, 0));
        myVehicleTypeDialog->myEditedDemandElement->setAttribute(SUMO_ATTR_SPEEDFACTOR, myTextFieldSpeedFactor->getText().text(), myVehicleTypeDialog->myEditedDemandElement->getViewNet()->getUndoList());
    } else {
        myTextFieldSpeedFactor->setTextColor(FXRGB(255, 0, 0));
        myVehicleTypeDialog->myVehicleTypeValid = false;
        myVehicleTypeDialog->myInvalidAttr = SUMO_ATTR_SPEEDFACTOR;
    }
    // set color of myTextFieldSpeedDev, depending if current value is valid or not
    if (myVehicleTypeDialog->myEditedDemandElement->isValid(SUMO_ATTR_SPEEDDEV, myTextFieldSpeedDev->getText().text())) {
        myTextFieldSpeedDev->setTextColor(FXRGB(0, 0, 0));
        myVehicleTypeDialog->myEditedDemandElement->setAttribute(SUMO_ATTR_SPEEDDEV, myTextFieldSpeedDev->getText().text(), myVehicleTypeDialog->myEditedDemandElement->getViewNet()->getUndoList());
    } else {
        myTextFieldSpeedDev->setTextColor(FXRGB(255, 0, 0));
        myVehicleTypeDialog->myVehicleTypeValid = false;
        myVehicleTypeDialog->myInvalidAttr = SUMO_ATTR_SPEEDDEV;
    }
    // set color of myTextFieldColor, depending if current value is valid or not
    if (myVehicleTypeDialog->myEditedDemandElement->isValid(SUMO_ATTR_COLOR, myTextFieldColor->getText().text())) {
        myTextFieldColor->setTextColor(FXRGB(0, 0, 0));
        myVehicleTypeDialog->myEditedDemandElement->setAttribute(SUMO_ATTR_COLOR, myTextFieldColor->getText().text(), myVehicleTypeDialog->myEditedDemandElement->getViewNet()->getUndoList());
    } else {
        myTextFieldColor->setTextColor(FXRGB(255, 0, 0));
        myVehicleTypeDialog->myVehicleTypeValid = false;
        myVehicleTypeDialog->myInvalidAttr = SUMO_ATTR_COLOR;
    }
    // set color of myTextFieldEmissionClass, depending if current value is valid or not
    if (myVehicleTypeDialog->myEditedDemandElement->isValid(SUMO_ATTR_EMISSIONCLASS, myTextFieldEmissionClass->getText().text())) {
        myTextFieldEmissionClass->setTextColor(FXRGB(0, 0, 0));
        myVehicleTypeDialog->myEditedDemandElement->setAttribute(SUMO_ATTR_EMISSIONCLASS, myTextFieldEmissionClass->getText().text(), myVehicleTypeDialog->myEditedDemandElement->getViewNet()->getUndoList());
    } else {
        myTextFieldEmissionClass->setTextColor(FXRGB(255, 0, 0));
        myVehicleTypeDialog->myVehicleTypeValid = false;
        myVehicleTypeDialog->myInvalidAttr = SUMO_ATTR_EMISSIONCLASS;
    }
    // set color of myTextFieldWidth, depending if current value is valid or not
    if (myVehicleTypeDialog->myEditedDemandElement->isValid(SUMO_ATTR_WIDTH, myTextFieldWidth->getText().text())) {
        myTextFieldWidth->setTextColor(FXRGB(0, 0, 0));
        myVehicleTypeDialog->myEditedDemandElement->setAttribute(SUMO_ATTR_WIDTH, myTextFieldWidth->getText().text(), myVehicleTypeDialog->myEditedDemandElement->getViewNet()->getUndoList());
    } else {
        myTextFieldWidth->setTextColor(FXRGB(255, 0, 0));
        myVehicleTypeDialog->myVehicleTypeValid = false;
        myVehicleTypeDialog->myInvalidAttr = SUMO_ATTR_WIDTH;
    }
    // set color of myTextFieldFilename, depending if current value is valid or not
    if (myVehicleTypeDialog->myEditedDemandElement->isValid(SUMO_ATTR_IMGFILE, myTextFieldFilename->getText().text())) {
        myTextFieldFilename->setTextColor(FXRGB(0, 0, 0));
        myVehicleTypeDialog->myEditedDemandElement->setAttribute(SUMO_ATTR_IMGFILE, myTextFieldFilename->getText().text(), myVehicleTypeDialog->myEditedDemandElement->getViewNet()->getUndoList());
    } else {
        myTextFieldFilename->setTextColor(FXRGB(255, 0, 0));
        myVehicleTypeDialog->myVehicleTypeValid = false;
        myVehicleTypeDialog->myInvalidAttr = SUMO_ATTR_IMGFILE;
    }
    // set color of myTextFieldImpatience, depending if current value is valid or not
    if (myVehicleTypeDialog->myEditedDemandElement->isValid(SUMO_ATTR_IMPATIENCE, myTextFieldImpatience->getText().text())) {
        myTextFieldImpatience->setTextColor(FXRGB(0, 0, 0));
        myVehicleTypeDialog->myEditedDemandElement->setAttribute(SUMO_ATTR_IMPATIENCE, myTextFieldImpatience->getText().text(), myVehicleTypeDialog->myEditedDemandElement->getViewNet()->getUndoList());
    } else {
        myTextFieldImpatience->setTextColor(FXRGB(255, 0, 0));
        myVehicleTypeDialog->myVehicleTypeValid = false;
        myVehicleTypeDialog->myInvalidAttr = SUMO_ATTR_IMPATIENCE;
    }
    // set color of myTextFieldLaneChangeModel, depending if current value is valid or not
    if (myVehicleTypeDialog->myEditedDemandElement->isValid(SUMO_ATTR_LANE_CHANGE_MODEL, myTextFieldLaneChangeModel->getText().text())) {
        myTextFieldLaneChangeModel->setTextColor(FXRGB(0, 0, 0));
        myVehicleTypeDialog->myEditedDemandElement->setAttribute(SUMO_ATTR_LANE_CHANGE_MODEL, myTextFieldLaneChangeModel->getText().text(), myVehicleTypeDialog->myEditedDemandElement->getViewNet()->getUndoList());
    } else {
        myTextFieldLaneChangeModel->setTextColor(FXRGB(255, 0, 0));
        myVehicleTypeDialog->myVehicleTypeValid = false;
        myVehicleTypeDialog->myInvalidAttr = SUMO_ATTR_LANE_CHANGE_MODEL;
    }
    // set color of myTextFieldPersonCapacity, depending if current value is valid or not
    if (myVehicleTypeDialog->myEditedDemandElement->isValid(SUMO_ATTR_PERSON_CAPACITY, myTextFieldPersonCapacity->getText().text())) {
        myTextFieldPersonCapacity->setTextColor(FXRGB(0, 0, 0));
        myVehicleTypeDialog->myEditedDemandElement->setAttribute(SUMO_ATTR_PERSON_CAPACITY, myTextFieldPersonCapacity->getText().text(), myVehicleTypeDialog->myEditedDemandElement->getViewNet()->getUndoList());
    } else {
        myTextFieldPersonCapacity->setTextColor(FXRGB(255, 0, 0));
        myVehicleTypeDialog->myVehicleTypeValid = false;
        myVehicleTypeDialog->myInvalidAttr = SUMO_ATTR_PERSON_CAPACITY;
    }
    // set color of myTextFieldContainerCapacity, depending if current value is valid or not
    if (myVehicleTypeDialog->myEditedDemandElement->isValid(SUMO_ATTR_CONTAINER_CAPACITY, myTextFieldContainerCapacity->getText().text())) {
        myTextFieldContainerCapacity->setTextColor(FXRGB(0, 0, 0));
        myVehicleTypeDialog->myEditedDemandElement->setAttribute(SUMO_ATTR_CONTAINER_CAPACITY, myTextFieldContainerCapacity->getText().text(), myVehicleTypeDialog->myEditedDemandElement->getViewNet()->getUndoList());
    } else {
        myTextFieldContainerCapacity->setTextColor(FXRGB(255, 0, 0));
        myVehicleTypeDialog->myVehicleTypeValid = false;
        myVehicleTypeDialog->myInvalidAttr = SUMO_ATTR_CONTAINER_CAPACITY;
    }
    // set color of myTextFieldBoardingDuration, depending if current value is valid or not
    if (myVehicleTypeDialog->myEditedDemandElement->isValid(SUMO_ATTR_BOARDING_DURATION, myTextFieldBoardingDuration->getText().text())) {
        myTextFieldBoardingDuration->setTextColor(FXRGB(0, 0, 0));
        myVehicleTypeDialog->myEditedDemandElement->setAttribute(SUMO_ATTR_BOARDING_DURATION, myTextFieldBoardingDuration->getText().text(), myVehicleTypeDialog->myEditedDemandElement->getViewNet()->getUndoList());
    } else {
        myTextFieldBoardingDuration->setTextColor(FXRGB(255, 0, 0));
        myVehicleTypeDialog->myVehicleTypeValid = false;
        myVehicleTypeDialog->myInvalidAttr = SUMO_ATTR_BOARDING_DURATION;
    }
    // set color of myTextFieldLoadingDuration, depending if current value is valid or not
    if (myVehicleTypeDialog->myEditedDemandElement->isValid(SUMO_ATTR_LOADING_DURATION, myTextFieldLoadingDuration->getText().text())) {
        myTextFieldLoadingDuration->setTextColor(FXRGB(0, 0, 0));
        myVehicleTypeDialog->myEditedDemandElement->setAttribute(SUMO_ATTR_LOADING_DURATION, myTextFieldLoadingDuration->getText().text(), myVehicleTypeDialog->myEditedDemandElement->getViewNet()->getUndoList());
    } else {
        myTextFieldLoadingDuration->setTextColor(FXRGB(255, 0, 0));
        myVehicleTypeDialog->myVehicleTypeValid = false;
        myVehicleTypeDialog->myInvalidAttr = SUMO_ATTR_LOADING_DURATION;
    }
    // set color of myTextFieldLatAlignment, depending if current value is valid or not
    if (myVehicleTypeDialog->myEditedDemandElement->isValid(SUMO_ATTR_LATALIGNMENT, myTextFieldLatAlignment->getText().text())) {
        myTextFieldLatAlignment->setTextColor(FXRGB(0, 0, 0));
        myVehicleTypeDialog->myEditedDemandElement->setAttribute(SUMO_ATTR_LATALIGNMENT, myTextFieldLatAlignment->getText().text(), myVehicleTypeDialog->myEditedDemandElement->getViewNet()->getUndoList());
    } else {
        myTextFieldLatAlignment->setTextColor(FXRGB(255, 0, 0));
        myVehicleTypeDialog->myVehicleTypeValid = false;
        myVehicleTypeDialog->myInvalidAttr = SUMO_ATTR_LATALIGNMENT;
    }
    // set color of myTextFieldMinGapLat, depending if current value is valid or not
    if (myVehicleTypeDialog->myEditedDemandElement->isValid(SUMO_ATTR_MINGAP, myTextFieldMinGapLat->getText().text())) {
        myTextFieldMinGapLat->setTextColor(FXRGB(0, 0, 0));
        myVehicleTypeDialog->myEditedDemandElement->setAttribute(SUMO_ATTR_MINGAP, myTextFieldMinGapLat->getText().text(), myVehicleTypeDialog->myEditedDemandElement->getViewNet()->getUndoList());
    } else {
        myTextFieldMinGapLat->setTextColor(FXRGB(255, 0, 0));
        myVehicleTypeDialog->myVehicleTypeValid = false;
        myVehicleTypeDialog->myInvalidAttr = SUMO_ATTR_MINGAP_LAT;
    }
    // set color of myTextFieldVehicleTypeID, depending if current value is valid or not
    if (myVehicleTypeDialog->myEditedDemandElement->isValid(SUMO_ATTR_MAXSPEED, myTextFieldMaxSpeedLat->getText().text())) {
        myTextFieldMaxSpeedLat->setTextColor(FXRGB(0, 0, 0));
        myVehicleTypeDialog->myEditedDemandElement->setAttribute(SUMO_ATTR_MAXSPEED, myTextFieldMaxSpeedLat->getText().text(), myVehicleTypeDialog->myEditedDemandElement->getViewNet()->getUndoList());
    } else {
        myTextFieldMaxSpeedLat->setTextColor(FXRGB(255, 0, 0));
        myVehicleTypeDialog->myVehicleTypeValid = false;
        myVehicleTypeDialog->myInvalidAttr = SUMO_ATTR_MAXSPEED_LAT;
    }
}


void 
GNEVehicleTypeDialog::VTypeCommonAtributes::updateValues() {
    // set variables of special rows VType and VShape
    myVClassRow->updateValues();
    myVShapeRow->updateValues();
    //set values of myEditedDemandElement into fields
    myTextFieldVehicleTypeID->setText(myVehicleTypeDialog->myEditedDemandElement->getAttribute(SUMO_ATTR_ID).c_str());
    myTextFieldLength->setText(myVehicleTypeDialog->myEditedDemandElement->getAttribute(SUMO_ATTR_LENGTH).c_str());
    myTextFieldMinGap->setText(myVehicleTypeDialog->myEditedDemandElement->getAttribute(SUMO_ATTR_MINGAP).c_str());
    myTextFieldMaxSpeed->setText(myVehicleTypeDialog->myEditedDemandElement->getAttribute(SUMO_ATTR_MAXSPEED).c_str());
    myTextFieldSpeedFactor->setText(myVehicleTypeDialog->myEditedDemandElement->getAttribute(SUMO_ATTR_SPEEDFACTOR).c_str());
    myTextFieldSpeedDev->setText(myVehicleTypeDialog->myEditedDemandElement->getAttribute(SUMO_ATTR_SPEEDDEV).c_str());
    myTextFieldColor->setText(myVehicleTypeDialog->myEditedDemandElement->getAttribute(SUMO_ATTR_COLOR).c_str());
    myTextFieldEmissionClass->setText(myVehicleTypeDialog->myEditedDemandElement->getAttribute(SUMO_ATTR_EMISSIONCLASS).c_str());
    myTextFieldWidth->setText(myVehicleTypeDialog->myEditedDemandElement->getAttribute(SUMO_ATTR_WIDTH).c_str());
    myTextFieldFilename->setText(myVehicleTypeDialog->myEditedDemandElement->getAttribute(SUMO_ATTR_IMGFILE).c_str());
    myTextFieldImpatience->setText(myVehicleTypeDialog->myEditedDemandElement->getAttribute(SUMO_ATTR_IMPATIENCE).c_str());
    myTextFieldLaneChangeModel->setText(myVehicleTypeDialog->myEditedDemandElement->getAttribute(SUMO_ATTR_LANE_CHANGE_MODEL).c_str());
    myTextFieldPersonCapacity->setText(myVehicleTypeDialog->myEditedDemandElement->getAttribute(SUMO_ATTR_PERSON_CAPACITY).c_str());
    myTextFieldContainerCapacity->setText(myVehicleTypeDialog->myEditedDemandElement->getAttribute(SUMO_ATTR_CONTAINER_CAPACITY).c_str());
    myTextFieldBoardingDuration->setText(myVehicleTypeDialog->myEditedDemandElement->getAttribute(SUMO_ATTR_BOARDING_DURATION).c_str());
    myTextFieldLoadingDuration->setText(myVehicleTypeDialog->myEditedDemandElement->getAttribute(SUMO_ATTR_LOADING_DURATION).c_str());
    myTextFieldLatAlignment->setText(myVehicleTypeDialog->myEditedDemandElement->getAttribute(SUMO_ATTR_LATALIGNMENT).c_str());
    myTextFieldMinGapLat->setText(myVehicleTypeDialog->myEditedDemandElement->getAttribute(SUMO_ATTR_MINGAP_LAT).c_str());
    myTextFieldMaxSpeedLat->setText(myVehicleTypeDialog->myEditedDemandElement->getAttribute(SUMO_ATTR_MAXSPEED_LAT).c_str());
}

// ---------------------------------------------------------------------------
// GNEVehicleTypeDialog::VShapeRow - methods
// ---------------------------------------------------------------------------

GNEVehicleTypeDialog::CarFollowingModelParameters::CarFollowingModelParameters(GNEVehicleTypeDialog* vehicleTypeDialog, FXHorizontalFrame* column) :
    FXGroupBox(column, "Car Following Model", GUIDesignGroupBoxFrame),
    myVehicleTypeDialog(vehicleTypeDialog) {

    // create vertical frame for rows
    myVerticalFrameRows = new FXVerticalFrame(this, GUIDesignAuxiliarFrame);

    // declare combo box
    FXHorizontalFrame* row = new FXHorizontalFrame(myVerticalFrameRows, GUIDesignAuxiliarHorizontalFrame);
    new FXLabel(row, "Algorithm", nullptr, GUIDesignLabelAttribute150);
    myComboBoxCarFollowModel = new FXComboBox(row, GUIDesignComboBoxNCol, vehicleTypeDialog, MID_GNE_CALIBRATORDIALOG_SET_VARIABLE, GUIDesignComboBox);
    
    // fill combo Box with all Car following models
    std::vector<std::string> CFModels = SUMOXMLDefinitions::CarFollowModels.getStrings();
    for (auto i : CFModels) {
        myComboBoxCarFollowModel->appendItem(i.c_str());
    }
    myComboBoxCarFollowModel->setNumVisible(10);

    // 01 create FXTextField and Label for Accel
    myTextFieldAccel = new CarFollowingModelRow(this, myVerticalFrameRows, SUMO_ATTR_ACCEL);

    // 02 create FXTextField and Label for Decel
    myTextFieldDecel = new CarFollowingModelRow(this, myVerticalFrameRows, SUMO_ATTR_DECEL);

    // 03 create FXTextField and Label for Apparent decel
    myTextFieldApparentDecel = new CarFollowingModelRow(this, myVerticalFrameRows, SUMO_ATTR_APPARENTDECEL);

    // 04 create FXTextField and Label for emergency decel
    myTextFieldEmergencyDecel = new CarFollowingModelRow(this, myVerticalFrameRows, SUMO_ATTR_EMERGENCYDECEL);

    // 05 create FXTextField and Label for Sigma
    myTextFieldSigma = new CarFollowingModelRow(this, myVerticalFrameRows, SUMO_ATTR_SIGMA);

    // 06 create FXTextField and Label for Tau
    myTextFieldTau = new CarFollowingModelRow(this, myVerticalFrameRows, SUMO_ATTR_TAU);

    // 07 myTextFieldMinGapFactor FXTextField and Label for MinGapFactor
    myTextFieldMinGapFactor = new CarFollowingModelRow(this, myVerticalFrameRows, SUMO_ATTR_COLLISION_MINGAP_FACTOR);

    // 08 create FXTextField and Label for K
    myTextFieldK = new CarFollowingModelRow(this, myVerticalFrameRows, SUMO_ATTR_K);

    // 09 create FXTextField and Label for PHI
    myTextFieldPhi = new CarFollowingModelRow(this, myVerticalFrameRows, SUMO_ATTR_CF_KERNER_PHI);

    // 10 create FXTextField and Label for Deleta
    myTextFieldDelta = new CarFollowingModelRow(this, myVerticalFrameRows, SUMO_ATTR_CF_IDM_DELTA);

    // 11 create FXTextField and Label for Stepping
    myTextFieldStepping = new CarFollowingModelRow(this, myVerticalFrameRows, SUMO_ATTR_CF_IDM_STEPPING);

    // 12 create FXTextField and Label for Security
    myTextFieldSecurity = new CarFollowingModelRow(this, myVerticalFrameRows, SUMO_ATTR_CF_WIEDEMANN_SECURITY);

    // 13 create FXTextField and Label for Estimation
    myTextFieldEstimation = new CarFollowingModelRow(this, myVerticalFrameRows, SUMO_ATTR_CF_WIEDEMANN_ESTIMATION);

    // 14 create FXTextField and Label for Estimation
    myTextFieldTmp1 = new CarFollowingModelRow(this, myVerticalFrameRows, SUMO_ATTR_TMP1);

    // 15 create FXTextField and Label for Estimation
    myTextFieldTmp2 = new CarFollowingModelRow(this, myVerticalFrameRows, SUMO_ATTR_TMP2);

    // 16 create FXTextField and Label for Estimation
    myTextFieldTmp3 = new CarFollowingModelRow(this, myVerticalFrameRows, SUMO_ATTR_TMP3);

    // 17 create FXTextField and Label for Estimation
    myTextFieldTmp4 = new CarFollowingModelRow(this, myVerticalFrameRows, SUMO_ATTR_TMP4);

    // 18 create FXTextField and Label for Estimation
    myTextFieldTmp5 = new CarFollowingModelRow(this, myVerticalFrameRows, SUMO_ATTR_TMP5);

    // 19 create FXTextField and Label for Estimation
    myTextFieldTrainType = new CarFollowingModelRow(this, myVerticalFrameRows, SUMO_ATTR_TRAIN_TYPE);

    // show or hidde ComboBox depending of current selected CFM
    refreshCFMFields();
}


void 
GNEVehicleTypeDialog::CarFollowingModelParameters::refreshCFMFields() {
    // start hidding all textfields (except TAU, because it's common for all CFM)
    myTextFieldAccel->hide();
    myTextFieldDecel->hide();
    myTextFieldApparentDecel->hide();
    myTextFieldEmergencyDecel->hide();
    myTextFieldSigma->hide();
    myTextFieldMinGapFactor->hide();
    myTextFieldK->hide();
    myTextFieldPhi->hide();
    myTextFieldDelta->hide();
    myTextFieldStepping->hide();
    myTextFieldSecurity->hide();
    myTextFieldEstimation->hide();
    myTextFieldTmp1->hide();
    myTextFieldTmp2->hide();
    myTextFieldTmp3->hide();
    myTextFieldTmp4->hide();
    myTextFieldTmp5->hide();
    myTextFieldTrainType->hide();
    // show textfield depending of current CFM
    if (SUMOXMLDefinitions::CarFollowModels.hasString(myComboBoxCarFollowModel->getText().text())) {
        // show textfield depending of selected CFM
        switch (SUMOXMLDefinitions::CarFollowModels.get(myComboBoxCarFollowModel->getText().text())) {
            case SUMO_TAG_CF_KRAUSS:
            case SUMO_TAG_CF_KRAUSS_ORIG1:
            case SUMO_TAG_CF_KRAUSS_PLUS_SLOPE:
                myTextFieldAccel->show();
                myTextFieldDecel->show();
                myTextFieldApparentDecel->show();
                myTextFieldEmergencyDecel->show();
                myTextFieldSigma->show();
                break;
            case SUMO_TAG_CF_KRAUSSX:
                myTextFieldTmp1->show();
                myTextFieldTmp2->show();
                myTextFieldTmp3->show();
                myTextFieldTmp4->show();
                myTextFieldTmp5->show();
                break;
            case SUMO_TAG_CF_SMART_SK:
            case SUMO_TAG_CF_DANIEL1:
                myTextFieldAccel->show();
                myTextFieldDecel->show();
                myTextFieldEmergencyDecel->show();
                myTextFieldSigma->show();
                myTextFieldMinGapFactor->show();
                myTextFieldTmp1->show();
                myTextFieldTmp2->show();
                myTextFieldTmp3->show();
                myTextFieldTmp4->show();
                myTextFieldTmp5->show();
                break;
            case SUMO_TAG_CF_PWAGNER2009:
                myTextFieldAccel->show();
                myTextFieldDecel->show();
                myTextFieldEmergencyDecel->show();
                myTextFieldSigma->show();
                myTextFieldMinGapFactor->show();
                /** extra Fields for 
                    pwagParams.insert(SUMO_ATTR_CF_PWAGNER2009_TAULAST);
                    pwagParams.insert(SUMO_ATTR_CF_PWAGNER2009_APPROB);
                */
                break;
            case SUMO_TAG_CF_IDM:
                myTextFieldAccel->show();
                myTextFieldDecel->show();
                myTextFieldEmergencyDecel->show();
                myTextFieldStepping->show();
                myTextFieldMinGapFactor->show();
                break;
            case SUMO_TAG_CF_IDMM:
                myTextFieldAccel->show();
                myTextFieldDecel->show();
                myTextFieldEmergencyDecel->show();
                myTextFieldStepping->show();
                myTextFieldMinGapFactor->show();
                /** extra Fields for 
                    idmmParams.insert(SUMO_ATTR_CF_IDMM_ADAPT_FACTOR);
                    idmmParams.insert(SUMO_ATTR_CF_IDMM_ADAPT_TIME);
                */
                break;
            case SUMO_TAG_CF_BKERNER:
                myTextFieldAccel->show();
                myTextFieldDecel->show();
                myTextFieldEmergencyDecel->show();
                myTextFieldK->show();
                myTextFieldPhi->show();
                myTextFieldMinGapFactor->show();
                break;
            case SUMO_TAG_CF_WIEDEMANN: // Ask Jakob (Only two)?
                myTextFieldAccel->show();
                myTextFieldDecel->show();
                myTextFieldEmergencyDecel->show();  
                myTextFieldMinGapFactor->show();
                /**
                wiedemannParams.insert(SUMO_ATTR_CF_WIEDEMANN_SECURITY);
                wiedemannParams.insert(SUMO_ATTR_CF_WIEDEMANN_ESTIMATION);
                */
                break;
            case SUMO_TAG_CF_RAIL:
                myTextFieldTrainType->show();
                break;
            case SUMO_TAG_CF_ACC:
                myTextFieldAccel->show();
                myTextFieldDecel->show();
                myTextFieldEmergencyDecel->show();
                myTextFieldMinGapFactor->show();
                /**
                The follow parameters has to be inserted:
                    ACCParams.insert(SUMO_ATTR_SC_GAIN);
                    ACCParams.insert(SUMO_ATTR_GCC_GAIN_SPEED);
                    ACCParams.insert(SUMO_ATTR_GCC_GAIN_SPACE);
                    ACCParams.insert(SUMO_ATTR_GC_GAIN_SPEED);
                    ACCParams.insert(SUMO_ATTR_GC_GAIN_SPACE);
                    ACCParams.insert(SUMO_ATTR_CA_GAIN_SPEED);
                    ACCParams.insert(SUMO_ATTR_CA_GAIN_SPACE);
                */
                break;
            case SUMO_TAG_CF_CACC:
                myTextFieldAccel->show();
                myTextFieldDecel->show();
                myTextFieldEmergencyDecel->show();
                myTextFieldMinGapFactor->show();
                /**
                The follow parameters has to be inserted:
                    CACCParams.insert(SUMO_ATTR_SC_GAIN_CACC);
                    CACCParams.insert(SUMO_ATTR_GCC_GAIN_GAP_CACC);
                    CACCParams.insert(SUMO_ATTR_GCC_GAIN_GAP_DOT_CACC);
                    CACCParams.insert(SUMO_ATTR_GC_GAIN_GAP_CACC);
                    CACCParams.insert(SUMO_ATTR_GC_GAIN_GAP_DOT_CACC);
                    CACCParams.insert(SUMO_ATTR_CA_GAIN_GAP_CACC);
                    CACCParams.insert(SUMO_ATTR_CA_GAIN_GAP_DOT_CACC);
                    CACCParams.insert(SUMO_ATTR_GCC_GAIN_SPEED);
                    CACCParams.insert(SUMO_ATTR_GCC_GAIN_SPACE);
                    CACCParams.insert(SUMO_ATTR_GC_GAIN_SPEED);
                    CACCParams.insert(SUMO_ATTR_GC_GAIN_SPACE);
                    CACCParams.insert(SUMO_ATTR_CA_GAIN_SPEED);
                    CACCParams.insert(SUMO_ATTR_CA_GAIN_SPACE);
                */
                break;
            case SUMO_TAG_CF_CC:
                myTextFieldAccel->show();
                myTextFieldDecel->show();
                /**
                The follow parameters has to be inserted:
                    ccParams.insert(SUMO_ATTR_CF_CC_C1);
                    ccParams.insert(SUMO_ATTR_CF_CC_CCDECEL);
                    ccParams.insert(SUMO_ATTR_CF_CC_CONSTSPACING);
                    ccParams.insert(SUMO_ATTR_CF_CC_KP);
                    ccParams.insert(SUMO_ATTR_CF_CC_LAMBDA);
                    ccParams.insert(SUMO_ATTR_CF_CC_OMEGAN);
                    ccParams.insert(SUMO_ATTR_CF_CC_TAU);
                    ccParams.insert(SUMO_ATTR_CF_CC_XI);
                    ccParams.insert(SUMO_ATTR_CF_CC_LANES_COUNT);
                    ccParams.insert(SUMO_ATTR_CF_CC_CCACCEL);
                    ccParams.insert(SUMO_ATTR_CF_CC_PLOEG_KP);
                    ccParams.insert(SUMO_ATTR_CF_CC_PLOEG_KD);
                    ccParams.insert(SUMO_ATTR_CF_CC_PLOEG_H);
                    ccParams.insert(SUMO_ATTR_CF_CC_FLATBED_KA);
                    ccParams.insert(SUMO_ATTR_CF_CC_FLATBED_KV);
                    ccParams.insert(SUMO_ATTR_CF_CC_FLATBED_KP);
                    ccParams.insert(SUMO_ATTR_CF_CC_FLATBED_D);
                    ccParams.insert(SUMO_ATTR_CF_CC_FLATBED_H);
                */
                break;
            default:
                break;
        }
    }
    myVerticalFrameRows->recalc();
    update();
}


void 
GNEVehicleTypeDialog::CarFollowingModelParameters::setVariable() {
     // set color of myTextFieldCarFollowModel, depending if current value is valid or not
    if (myVehicleTypeDialog->myEditedDemandElement->isValid(SUMO_ATTR_CAR_FOLLOW_MODEL, myComboBoxCarFollowModel->getText().text())) {
        myComboBoxCarFollowModel->setTextColor(FXRGB(0, 0, 0));
        myVehicleTypeDialog->myEditedDemandElement->setAttribute(SUMO_ATTR_CAR_FOLLOW_MODEL, myComboBoxCarFollowModel->getText().text(), myVehicleTypeDialog->myEditedDemandElement->getViewNet()->getUndoList());
    } else {
        myComboBoxCarFollowModel->setTextColor(FXRGB(255, 0, 0));
        myVehicleTypeDialog->myVehicleTypeValid = false;
        myVehicleTypeDialog->myInvalidAttr = SUMO_ATTR_CAR_FOLLOW_MODEL;
    }
    // set color of myTextFieldAccel, depending if current value is valid or not
    if (myVehicleTypeDialog->myEditedDemandElement->isValid(SUMO_ATTR_ACCEL, myTextFieldAccel->textField->getText().text())) {
        myTextFieldAccel->textField->setTextColor(FXRGB(0, 0, 0));
        myVehicleTypeDialog->myEditedDemandElement->setAttribute(SUMO_ATTR_ACCEL, myTextFieldAccel->textField->getText().text(), myVehicleTypeDialog->myEditedDemandElement->getViewNet()->getUndoList());
    } else {
        myTextFieldAccel->textField->setTextColor(FXRGB(255, 0, 0));
        myVehicleTypeDialog->myVehicleTypeValid = false;
        myVehicleTypeDialog->myInvalidAttr = SUMO_ATTR_ACCEL;
    }
    // set color of myTextFieldDecel, depending if current value is valid or not
    if (myVehicleTypeDialog->myEditedDemandElement->isValid(SUMO_ATTR_DECEL, myTextFieldDecel->textField->getText().text())) {
        myTextFieldDecel->textField->setTextColor(FXRGB(0, 0, 0));
        myVehicleTypeDialog->myEditedDemandElement->setAttribute(SUMO_ATTR_DECEL, myTextFieldDecel->textField->getText().text(), myVehicleTypeDialog->myEditedDemandElement->getViewNet()->getUndoList());
    } else {
        myTextFieldDecel->textField->setTextColor(FXRGB(255, 0, 0));
        myVehicleTypeDialog->myVehicleTypeValid = false;
        myVehicleTypeDialog->myInvalidAttr = SUMO_ATTR_DECEL;
    }
    // set color of myTextFieldApparentDecel, depending if current value is valid or not
    if (myVehicleTypeDialog->myEditedDemandElement->isValid(SUMO_ATTR_APPARENTDECEL, myTextFieldApparentDecel->textField->getText().text())) {
        myTextFieldApparentDecel->textField->setTextColor(FXRGB(0, 0, 0));
        myVehicleTypeDialog->myEditedDemandElement->setAttribute(SUMO_ATTR_APPARENTDECEL, myTextFieldApparentDecel->textField->getText().text(), myVehicleTypeDialog->myEditedDemandElement->getViewNet()->getUndoList());
    } else {
        myTextFieldApparentDecel->textField->setTextColor(FXRGB(255, 0, 0));
        myVehicleTypeDialog->myVehicleTypeValid = false;
        myVehicleTypeDialog->myInvalidAttr = SUMO_ATTR_APPARENTDECEL;
    }
    // set color of myTextFieldEmergencyAccel, depending if current value is valid or not
    if (myVehicleTypeDialog->myEditedDemandElement->isValid(SUMO_ATTR_EMERGENCYDECEL, myTextFieldEmergencyDecel->textField->getText().text())) {
        myTextFieldEmergencyDecel->textField->setTextColor(FXRGB(0, 0, 0));
        myVehicleTypeDialog->myEditedDemandElement->setAttribute(SUMO_ATTR_EMERGENCYDECEL, myTextFieldEmergencyDecel->textField->getText().text(), myVehicleTypeDialog->myEditedDemandElement->getViewNet()->getUndoList());
    } else {
        myTextFieldEmergencyDecel->textField->setTextColor(FXRGB(255, 0, 0));
        myVehicleTypeDialog->myVehicleTypeValid = false;
        myVehicleTypeDialog->myInvalidAttr = SUMO_ATTR_EMERGENCYDECEL;
    }
    // set color of myTextFieldSigma, depending if current value is valid or not
    if (myVehicleTypeDialog->myEditedDemandElement->isValid(SUMO_ATTR_SIGMA, myTextFieldSigma->textField->getText().text())) {
        myTextFieldSigma->textField->setTextColor(FXRGB(0, 0, 0));
        myVehicleTypeDialog->myEditedDemandElement->setAttribute(SUMO_ATTR_SIGMA, myTextFieldSigma->textField->getText().text(), myVehicleTypeDialog->myEditedDemandElement->getViewNet()->getUndoList());
    } else {
        myTextFieldSigma->textField->setTextColor(FXRGB(255, 0, 0));
        myVehicleTypeDialog->myVehicleTypeValid = false;
        myVehicleTypeDialog->myInvalidAttr = SUMO_ATTR_SIGMA;
    }
    // set color of myTextFieldTau, depending if current value is valid or not
    if (myVehicleTypeDialog->myEditedDemandElement->isValid(SUMO_ATTR_TAU, myTextFieldTau->textField->getText().text())) {
        myTextFieldTau->textField->setTextColor(FXRGB(0, 0, 0));
        myVehicleTypeDialog->myEditedDemandElement->setAttribute(SUMO_ATTR_TAU, myTextFieldTau->textField->getText().text(), myVehicleTypeDialog->myEditedDemandElement->getViewNet()->getUndoList());
    } else {
        myTextFieldTau->textField->setTextColor(FXRGB(255, 0, 0));
        myVehicleTypeDialog->myVehicleTypeValid = false;
        myVehicleTypeDialog->myInvalidAttr = SUMO_ATTR_TAU;
    }
    // set color of myTextFieldMinGapFactor, depending if current value is valid or not
    if (myVehicleTypeDialog->myEditedDemandElement->isValid(SUMO_ATTR_COLLISION_MINGAP_FACTOR, myTextFieldMinGapFactor->textField->getText().text())) {
        myTextFieldMinGapFactor->textField->setTextColor(FXRGB(0, 0, 0));
        myVehicleTypeDialog->myEditedDemandElement->setAttribute(SUMO_ATTR_COLLISION_MINGAP_FACTOR, myTextFieldMinGapFactor->textField->getText().text(), myVehicleTypeDialog->myEditedDemandElement->getViewNet()->getUndoList());
    } else {
        myTextFieldMinGapFactor->textField->setTextColor(FXRGB(255, 0, 0));
        myVehicleTypeDialog->myVehicleTypeValid = false;
        myVehicleTypeDialog->myInvalidAttr = SUMO_ATTR_MINGAP;
    }
    // set color of myTextFieldTmp1, depending if current value is valid or not
    if (myVehicleTypeDialog->myEditedDemandElement->isValid(SUMO_ATTR_TMP1, myTextFieldTmp1->textField->getText().text())) {
        myTextFieldTmp1->textField->setTextColor(FXRGB(0, 0, 0));
        myVehicleTypeDialog->myEditedDemandElement->setAttribute(SUMO_ATTR_TMP1, myTextFieldTmp1->textField->getText().text(), myVehicleTypeDialog->myEditedDemandElement->getViewNet()->getUndoList());
    } else {
        myTextFieldTmp1->textField->setTextColor(FXRGB(255, 0, 0));
        myVehicleTypeDialog->myVehicleTypeValid = false;
        myVehicleTypeDialog->myInvalidAttr = SUMO_ATTR_TMP1;
    }
    // set color of myTextFieldTmp2, depending if current value is valid or not
    if (myVehicleTypeDialog->myEditedDemandElement->isValid(SUMO_ATTR_TMP2, myTextFieldTmp2->textField->getText().text())) {
        myTextFieldTmp2->textField->setTextColor(FXRGB(0, 0, 0));
        myVehicleTypeDialog->myEditedDemandElement->setAttribute(SUMO_ATTR_TMP2, myTextFieldTmp2->textField->getText().text(), myVehicleTypeDialog->myEditedDemandElement->getViewNet()->getUndoList());
    } else {
        myTextFieldTmp2->textField->setTextColor(FXRGB(255, 0, 0));
        myVehicleTypeDialog->myVehicleTypeValid = false;
        myVehicleTypeDialog->myInvalidAttr = SUMO_ATTR_TMP2;
    }
    // set color of myTextFieldTmp3, depending if current value is valid or not
    if (myVehicleTypeDialog->myEditedDemandElement->isValid(SUMO_ATTR_TMP3, myTextFieldTmp3->textField->getText().text())) {
        myTextFieldTmp3->textField->setTextColor(FXRGB(0, 0, 0));
        myVehicleTypeDialog->myEditedDemandElement->setAttribute(SUMO_ATTR_TMP3, myTextFieldTmp3->textField->getText().text(), myVehicleTypeDialog->myEditedDemandElement->getViewNet()->getUndoList());
    } else {
        myTextFieldTmp3->textField->setTextColor(FXRGB(255, 0, 0));
        myVehicleTypeDialog->myVehicleTypeValid = false;
        myVehicleTypeDialog->myInvalidAttr = SUMO_ATTR_TMP3;
    }
    // set color of myTextFieldTmp4, depending if current value is valid or not
    if (myVehicleTypeDialog->myEditedDemandElement->isValid(SUMO_ATTR_TMP4, myTextFieldTmp4->textField->getText().text())) {
        myTextFieldTmp4->textField->setTextColor(FXRGB(0, 0, 0));
        myVehicleTypeDialog->myEditedDemandElement->setAttribute(SUMO_ATTR_TMP4, myTextFieldTmp4->textField->getText().text(), myVehicleTypeDialog->myEditedDemandElement->getViewNet()->getUndoList());
    } else {
        myTextFieldTmp4->textField->setTextColor(FXRGB(255, 0, 0));
        myVehicleTypeDialog->myVehicleTypeValid = false;
        myVehicleTypeDialog->myInvalidAttr = SUMO_ATTR_TMP4;
    }
    // set color of myTextFieldTmp5, depending if current value is valid or not
    if (myVehicleTypeDialog->myEditedDemandElement->isValid(SUMO_ATTR_TMP5, myTextFieldTmp5->textField->getText().text())) {
        myTextFieldTmp5->textField->setTextColor(FXRGB(0, 0, 0));
        myVehicleTypeDialog->myEditedDemandElement->setAttribute(SUMO_ATTR_TMP5, myTextFieldTmp5->textField->getText().text(), myVehicleTypeDialog->myEditedDemandElement->getViewNet()->getUndoList());
    } else {
        myTextFieldTmp5->textField->setTextColor(FXRGB(255, 0, 0));
        myVehicleTypeDialog->myVehicleTypeValid = false;
        myVehicleTypeDialog->myInvalidAttr = SUMO_ATTR_TMP5;
    }
    // set color of myTextFieldTrainType, depending if current value is valid or not
    if (myVehicleTypeDialog->myEditedDemandElement->isValid(SUMO_ATTR_TRAIN_TYPE, myTextFieldTrainType->textField->getText().text())) {
        myTextFieldTrainType->textField->setTextColor(FXRGB(0, 0, 0));
        myVehicleTypeDialog->myEditedDemandElement->setAttribute(SUMO_ATTR_TRAIN_TYPE, myTextFieldTrainType->textField->getText().text(), myVehicleTypeDialog->myEditedDemandElement->getViewNet()->getUndoList());
    } else {
        myTextFieldTrainType->textField->setTextColor(FXRGB(255, 0, 0));
        myVehicleTypeDialog->myVehicleTypeValid = false;
        myVehicleTypeDialog->myInvalidAttr = SUMO_ATTR_TRAIN_TYPE;
    }
    // refresh fields
    refreshCFMFields();
}


void 
GNEVehicleTypeDialog::CarFollowingModelParameters::updateValues() {
    //set values of myEditedDemandElement into fields
    if (myVehicleTypeDialog->myEditedDemandElement->getAttribute(SUMO_ATTR_CAR_FOLLOW_MODEL).empty()) {
        myComboBoxCarFollowModel->setCurrentItem(0);
    } else {
        myComboBoxCarFollowModel->setText(myVehicleTypeDialog->myEditedDemandElement->getAttribute(SUMO_ATTR_CAR_FOLLOW_MODEL).c_str());
    }
    myTextFieldAccel->textField->setText(myVehicleTypeDialog->myEditedDemandElement->getAttribute(SUMO_ATTR_ACCEL).c_str());
    myTextFieldDecel->textField->setText(myVehicleTypeDialog->myEditedDemandElement->getAttribute(SUMO_ATTR_DECEL).c_str());
    myTextFieldApparentDecel->textField->setText(myVehicleTypeDialog->myEditedDemandElement->getAttribute(SUMO_ATTR_APPARENTDECEL).c_str());
    myTextFieldEmergencyDecel->textField->setText(myVehicleTypeDialog->myEditedDemandElement->getAttribute(SUMO_ATTR_EMERGENCYDECEL).c_str());
    myTextFieldSigma->textField->setText(myVehicleTypeDialog->myEditedDemandElement->getAttribute(SUMO_ATTR_SIGMA).c_str());
    myTextFieldTau->textField->setText(myVehicleTypeDialog->myEditedDemandElement->getAttribute(SUMO_ATTR_TAU).c_str());
    myTextFieldMinGapFactor->textField->setText(myVehicleTypeDialog->myEditedDemandElement->getAttribute(SUMO_ATTR_COLLISION_MINGAP_FACTOR).c_str());
    myTextFieldTmp1->textField->setText(myVehicleTypeDialog->myEditedDemandElement->getAttribute(SUMO_ATTR_TMP1).c_str());
    myTextFieldTmp2->textField->setText(myVehicleTypeDialog->myEditedDemandElement->getAttribute(SUMO_ATTR_TMP2).c_str());
    myTextFieldTmp3->textField->setText(myVehicleTypeDialog->myEditedDemandElement->getAttribute(SUMO_ATTR_TMP3).c_str());
    myTextFieldTmp4->textField->setText(myVehicleTypeDialog->myEditedDemandElement->getAttribute(SUMO_ATTR_TMP4).c_str());
    myTextFieldTmp5->textField->setText(myVehicleTypeDialog->myEditedDemandElement->getAttribute(SUMO_ATTR_TMP5).c_str());
    myTextFieldTrainType->textField->setText(myVehicleTypeDialog->myEditedDemandElement->getAttribute(SUMO_ATTR_TRAIN_TYPE).c_str());
}

// ---------------------------------------------------------------------------
// GNEVehicleTypeDialog - private methods
// ---------------------------------------------------------------------------

GNEVehicleTypeDialog::CarFollowingModelParameters::CarFollowingModelRow::CarFollowingModelRow(CarFollowingModelParameters *carFollowingModelParametersParent, FXVerticalFrame* verticalFrame, SumoXMLAttr attr) :
    FXHorizontalFrame(verticalFrame, GUIDesignAuxiliarHorizontalFrame) {
    myLabel = new FXLabel(this, toString(attr).c_str(), nullptr, GUIDesignLabelAttribute150);
    textField = new FXTextField(this, GUIDesignTextFieldNCol, carFollowingModelParametersParent, MID_GNE_CALIBRATORDIALOG_SET_VARIABLE, GUIDesignTextFieldReal);
}

// ---------------------------------------------------------------------------
// GNEVehicleTypeDialog - private methods
// ---------------------------------------------------------------------------

FXTextField* 
GNEVehicleTypeDialog::buildRowInt(FXPacker* column, SumoXMLAttr attr) {
    FXHorizontalFrame* row = new FXHorizontalFrame(column, GUIDesignAuxiliarHorizontalFrame);
    new FXLabel(row, toString(attr).c_str(), nullptr, GUIDesignLabelAttribute150);
    return new FXTextField(row, GUIDesignTextFieldNCol, this, MID_GNE_CALIBRATORDIALOG_SET_VARIABLE, GUIDesignTextFieldInt);
}


FXTextField * 
GNEVehicleTypeDialog::buildRowFloat(FXPacker* column, SumoXMLAttr attr) {
    FXHorizontalFrame* row = new FXHorizontalFrame(column, GUIDesignAuxiliarHorizontalFrame);
    new FXLabel(row, toString(attr).c_str(), nullptr, GUIDesignLabelAttribute150);
    return new FXTextField(row, GUIDesignTextFieldNCol, this, MID_GNE_CALIBRATORDIALOG_SET_VARIABLE, GUIDesignTextFieldReal);
}


FXTextField * 
GNEVehicleTypeDialog::buildRowString(FXPacker* column, SumoXMLAttr attr) {
    FXHorizontalFrame* row = new FXHorizontalFrame(column, GUIDesignAuxiliarHorizontalFrame);
    new FXLabel(row, toString(attr).c_str(), nullptr, GUIDesignLabelAttribute150);
    return new FXTextField(row, GUIDesignTextFieldNCol, this, MID_GNE_CALIBRATORDIALOG_SET_VARIABLE, GUIDesignTextField);
}


/****************************************************************************/

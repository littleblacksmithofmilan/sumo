/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2001-2019 German Aerospace Center (DLR) and others.
// This program and the accompanying materials
// are made available under the terms of the Eclipse Public License v2.0
// which accompanies this distribution, and is available at
// http://www.eclipse.org/legal/epl-v20.html
// SPDX-License-Identifier: EPL-2.0
/****************************************************************************/
/// @file    MSTransportable.h
/// @author  Michael Behrisch
/// @date    Tue, 21 Apr 2015
///
// The common superclass for modelling transportable objects like persons and containers
/****************************************************************************/
#pragma once

// ===========================================================================
// included modules
// ===========================================================================
#include <config.h>

#include <set>
#include <cassert>
#include <utils/common/SUMOTime.h>
#include <utils/common/SUMOVehicleClass.h>
#include <utils/geom/Position.h>
#include <utils/geom/PositionVector.h>
#include <utils/geom/Boundary.h>
#include <utils/router/SUMOAbstractRouter.h>
#include <utils/vehicle/SUMOTrafficObject.h>


// ===========================================================================
// class declarations
// ===========================================================================
class MSEdge;
class MSLane;
class MSNet;
class MSStoppingPlace;
class MSVehicleType;
class OutputDevice;
class SUMOVehicleParameter;
class SUMOVehicle;
class MSTransportableDevice;
class MSTransportable;

typedef std::vector<const MSEdge*> ConstMSEdgeVector;

// ===========================================================================
// class definitions
// ===========================================================================
enum class MSStageType {
    WAITING_FOR_DEPART = 0,
    WAITING = 1,
    WALKING = 2, // only for persons
    DRIVING = 3,
    ACCESS = 4,
    TRIP = 5,
    TRANSHIP = 6
};

/**
* The "abstract" class for a single stage of a movement
* Contains the destination of the current movement step
*/
class MSStage {
public:
    /// constructor
    MSStage(const MSEdge* destination, MSStoppingPlace* toStop, const double arrivalPos, MSStageType type);

    /// destructor
    virtual ~MSStage();

    /// returns the destination edge
    const MSEdge* getDestination() const;

    /// returns the destination stop (if any)
    MSStoppingPlace* getDestinationStop() const {
        return myDestinationStop;
    }

    /// returns the origin stop (if any). only needed for MSStageTrip
    virtual const MSStoppingPlace* getOriginStop() const {
        return nullptr;
    }

    double getArrivalPos() const {
        return myArrivalPos;
    }

    /// Returns the current edge
    virtual const MSEdge* getEdge() const;
    virtual const MSEdge* getFromEdge() const;
    virtual double getEdgePos(SUMOTime now) const;

    /// returns the position of the transportable
    virtual Position getPosition(SUMOTime now) const = 0;

    /// returns the angle of the transportable
    virtual double getAngle(SUMOTime now) const = 0;

    ///
    MSStageType getStageType() const {
        return myType;
    }

    /// @brief return (brief) string representation of the current stage
    virtual std::string getStageDescription() const = 0;

    /// @brief return string summary of the current stage
    virtual std::string getStageSummary() const = 0;

    /// proceeds to this stage
    virtual void proceed(MSNet* net, MSTransportable* transportable, SUMOTime now, MSStage* previous) = 0;

    /// abort this stage (TraCI)
    virtual void abort(MSTransportable*) {};

    /// sets the walking speed (ignored in other stages)
    virtual void setSpeed(double) {};

    /// get departure time of stage
    SUMOTime getDeparted() const;

    /// logs end of the step
    void setDeparted(SUMOTime now);

    /// logs end of the step
    virtual const std::string& setArrived(MSNet* net, MSTransportable* transportable, SUMOTime now);

    /// Whether the transportable waits for the given vehicle
    virtual bool isWaitingFor(const SUMOVehicle* vehicle) const;

    /// @brief Whether the transportable waits for a vehicle
    virtual bool isWaiting4Vehicle() const {
        return false;
    }

    /// @brief Whether the transportable waits for a vehicle
    virtual SUMOVehicle* getVehicle() const {
        return nullptr;
    }

    /// @brief the time this transportable spent waiting
    virtual SUMOTime getWaitingTime(SUMOTime now) const;

    /// @brief the speed of the transportable
    virtual double getSpeed() const;

    /// @brief the edges of the current stage
    virtual ConstMSEdgeVector getEdges() const;

    /// @brief get position on edge e at length at with orthogonal offset
    Position getEdgePosition(const MSEdge* e, double at, double offset) const;

    /// @brief get position on lane at length at with orthogonal offset
    Position getLanePosition(const MSLane* lane, double at, double offset) const;

    /// @brief get angle of the edge at a certain position
    double getEdgeAngle(const MSEdge* e, double at) const;

    void setDestination(const MSEdge* newDestination, MSStoppingPlace* newDestStop);

    /// @brief get travel distance in this stage
    virtual double getDistance() const = 0;

    /** @brief Called on writing tripinfo output
     * @param[in] os The stream to write the information into
     * @exception IOError not yet implemented
     */
    virtual void tripInfoOutput(OutputDevice& os, const MSTransportable* const transportable) const = 0;

    /** @brief Called on writing vehroute output
     * @param[in] os The stream to write the information into
     * @param[in] withRouteLength whether route length shall be written
     * @exception IOError not yet implemented
     */
    virtual void routeOutput(OutputDevice& os, const bool withRouteLength) const = 0;

    virtual MSStage* clone() const = 0;

protected:
    /// the next edge to reach by getting transported
    const MSEdge* myDestination;

    /// the stop to reach by getting transported (if any)
    MSStoppingPlace* myDestinationStop;

    /// the position at which we want to arrive
    double myArrivalPos;

    /// the time at which this stage started
    SUMOTime myDeparted;

    /// the time at which this stage ended
    SUMOTime myArrived;

    /// The type of this stage
    MSStageType myType;

    /// @brief the offset for computing positions when standing at an edge
    static const double ROADSIDE_OFFSET;

private:
    /// @brief Invalidated copy constructor.
    MSStage(const MSStage&);

    /// @brief Invalidated assignment operator.
    MSStage& operator=(const MSStage&) = delete;

};

/**
* A "placeholder" stage storing routing info which will result in real stages when routed
*/
class MSStageTrip : public MSStage {
public:
    /// constructor
    MSStageTrip(const MSEdge* origin, MSStoppingPlace* fromStop,
                const MSEdge* destination, MSStoppingPlace* toStop,
                const SUMOTime duration, const SVCPermissions modeSet,
                const std::string& vTypes, const double speed, const double walkFactor,
                const double departPosLat, const bool hasArrivalPos, const double arrivalPos);

    /// destructor
    virtual ~MSStageTrip();

    MSStage* clone() const;

    const MSEdge* getEdge() const;

    const MSStoppingPlace* getOriginStop() const {
        return myOriginStop;
    }

    double getEdgePos(SUMOTime now) const;

    Position getPosition(SUMOTime now) const;

    double getAngle(SUMOTime now) const;

    double getDistance() const {
        // invalid
        return -1;
    }

    std::string getStageDescription() const {
        return "trip";
    }

    std::string getStageSummary() const;

    /// logs end of the step
    virtual const std::string& setArrived(MSNet* net, MSTransportable* transportable, SUMOTime now);

    /// change origin for parking area rerouting
    void setOrigin(const MSEdge* origin) {
        myOrigin = origin;
    }

    /// proceeds to the next step
    virtual void proceed(MSNet* net, MSTransportable* transportable, SUMOTime now, MSStage* previous);

    /** @brief Called on writing tripinfo output
    *
    * @param[in] os The stream to write the information into
    * @exception IOError not yet implemented
    */
    virtual void tripInfoOutput(OutputDevice& os, const MSTransportable* const transportable) const;

    /** @brief Called on writing vehroute output
    *
    * @param[in] os The stream to write the information into
    * @exception IOError not yet implemented
    */
    virtual void routeOutput(OutputDevice& os, const bool withRouteLength) const;

private:
    /// the origin edge
    const MSEdge* myOrigin;

    /// the origin edge
    const MSStoppingPlace* myOriginStop;

    /// the time the trip should take (applies to only walking)
    SUMOTime myDuration;

    /// @brief The allowed modes of transportation
    const SVCPermissions myModeSet;

    /// @brief The possible vehicles to use
    const std::string myVTypes;

    /// @brief The walking speed
    const double mySpeed;

    /// @brief The factor to apply to walking durations
    const double myWalkFactor;

    /// @brief The depart position
    double myDepartPos;

    /// @brief The lateral depart position
    const double myDepartPosLat;

    /// @brief whether an arrivalPos was in the input
    const bool myHaveArrivalPos;

private:
    /// @brief Invalidated copy constructor.
    MSStageTrip(const MSStageTrip&);

    /// @brief Invalidated assignment operator.
    MSStageTrip& operator=(const MSStageTrip&);

};

/**
* A "real" stage performing a waiting over the specified time
*/
class MSStageWaiting : public MSStage {
public:
    /// constructor
    MSStageWaiting(const MSEdge* destination, MSStoppingPlace* toStop, SUMOTime duration, SUMOTime until,
                    double pos, const std::string& actType, const bool initial);

    /// destructor
    virtual ~MSStageWaiting();

    MSStage* clone() const;

    /// abort this stage (TraCI)
    void abort(MSTransportable*);

    SUMOTime getUntil() const;

    ///
    Position getPosition(SUMOTime now) const;

    double getAngle(SUMOTime now) const;

    /// @brief get travel distance in this stage
    double getDistance() const {
        return 0;
    }

    SUMOTime getWaitingTime(SUMOTime now) const;

    std::string getStageDescription() const {
        return "waiting (" + myActType + ")";
    }

    std::string getStageSummary() const;

    /// proceeds to the next step
    virtual void proceed(MSNet* net, MSTransportable* transportable, SUMOTime now, MSStage* previous);

    /** @brief Called on writing tripinfo output
    *
    * @param[in] os The stream to write the information into
    * @exception IOError not yet implemented
    */
    virtual void tripInfoOutput(OutputDevice& os, const MSTransportable* const transportable) const;

    /** @brief Called on writing vehroute output
    *
    * @param[in] os The stream to write the information into
    * @exception IOError not yet implemented
    */
    virtual void routeOutput(OutputDevice& os, const bool withRouteLength) const;

private:
    /// the time the person is waiting
    SUMOTime myWaitingDuration;

    /// the time until the person is waiting
    SUMOTime myWaitingUntil;

    /// @brief The type of activity
    std::string myActType;

private:
    /// @brief Invalidated copy constructor.
    MSStageWaiting(const MSStageWaiting&);

    /// @brief Invalidated assignment operator.
    MSStageWaiting& operator=(const MSStageWaiting&);

};

/**
* A "real" stage performing the travelling by a transport system
* The given route will be chosen. The travel time is computed by the simulation
*/
class MSStageDriving : public MSStage {
public:
    /// constructor
    MSStageDriving(const MSEdge* destination, MSStoppingPlace* toStop,
                   const double arrivalPos, const std::vector<std::string>& lines,
                   const std::string& intendedVeh = "", SUMOTime intendedDepart = -1);

    /// destructor
    virtual ~MSStageDriving();

    /// abort this stage (TraCI)
    void abort(MSTransportable*);

    /// Returns the current edge
    const MSEdge* getEdge() const;
    const MSEdge* getFromEdge() const;
    double getEdgePos(SUMOTime now) const;

    ///
    Position getPosition(SUMOTime now) const;

    double getAngle(SUMOTime now) const;

    /// @brief get travel distance in this stage
    double getDistance() const {
        return myVehicleDistance;
    }

    /// Whether the person waits for the given vehicle
    bool isWaitingFor(const SUMOVehicle* vehicle) const;

    /// @brief Whether the person waits for a vehicle
    bool isWaiting4Vehicle() const;

    /// @brief Return where the person waits and for what
    std::string getWaitingDescription() const;

    SUMOVehicle* getVehicle() const {
        return myVehicle;
    }

    /// @brief time spent waiting for a ride
    SUMOTime getWaitingTime(SUMOTime now) const;

    double getSpeed() const;

    ConstMSEdgeVector getEdges() const;

    void setVehicle(SUMOVehicle* v);

    /// @brief marks arrival time and records driven distance
    const std::string& setArrived(MSNet* net, MSTransportable* transportable, SUMOTime now);

    const std::set<std::string>& getLines() const {
        return myLines;
    }

    std::string getIntendedVehicleID() const {
        return myIntendedVehicleID;
    }

    SUMOTime getIntendedDepart() const {
        return myIntendedDepart;
    }

protected:
    /// the lines  to choose from
    const std::set<std::string> myLines;

    /// @brief The taken vehicle
    SUMOVehicle* myVehicle;
    /// @brief cached vehicle data for output after the vehicle has been removed
    std::string myVehicleID;
    std::string myVehicleLine;

    SUMOVehicleClass myVehicleVClass;
    double myVehicleDistance;

    double myWaitingPos;
    /// @brief The time since which this person is waiting for a ride
    SUMOTime myWaitingSince;
    const MSEdge* myWaitingEdge;
    Position myStopWaitPos;

    std::string myIntendedVehicleID;
    SUMOTime myIntendedDepart;

private:
    /// @brief Invalidated copy constructor.
    MSStageDriving(const MSStageDriving&);

    /// @brief Invalidated assignment operator.
    MSStageDriving& operator=(const MSStageDriving&) = delete;

};


/****************************************************************************/

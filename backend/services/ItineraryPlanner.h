#pragma once

#include "utils/Http.h"

#include <string>
#include <vector>

struct PlannerPlace
{
    std::string name;
    std::string description;
    std::string category;
    std::string visitTime;
    double baseCost = 0;
    std::string transport;
};

struct PlannerDay
{
    int dayNumber = 0;
    std::string date;
    std::string title;
    std::string description;
    std::string meals;
    std::string accommodation;
    std::string transportation;
    double dailyBudget = 0;
    std::vector<PlannerPlace> places;
};

struct PlannerInput
{
    std::string destination;
    std::string startDate;
    int durationDays = 1;
    double budget = 0;
    int members = 1;
    std::string travelStyle = "standard";
    std::string accommodationPreference;
    std::string transportationPreference;
    std::string dietaryPreferences;
    std::vector<std::string> interests;
};

struct PlannerResult
{
    std::vector<PlannerDay> days;
    http::Json budgetAllocation;
    double estimatedTotal = 0;
    std::string source = "rule-based-catalog";
    std::string notes;
};

class ItineraryPlanner
{
public:
    PlannerResult generate(const PlannerInput& input) const;
};

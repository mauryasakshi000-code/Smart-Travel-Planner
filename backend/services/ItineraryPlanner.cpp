#include "services/ItineraryPlanner.h"

#include <algorithm>
#include <cmath>
#include <ctime>
#include <cctype>
#include <iomanip>
#include <map>
#include <sstream>

namespace
{
std::string lowerCopy(std::string value)
{
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return value;
}

std::string addDays(const std::string& isoDate, int offset)
{
    if (isoDate.size() != 10)
    {
        return {};
    }
    std::tm tm{};
    std::istringstream in(isoDate);
    in >> std::get_time(&tm, "%Y-%m-%d");
    if (in.fail())
    {
        return {};
    }
    tm.tm_mday += offset;
    tm.tm_isdst = -1;
    const auto time = std::mktime(&tm);
    if (time == -1)
    {
        return {};
    }
    std::ostringstream out;
    out << std::put_time(std::localtime(&time), "%Y-%m-%d");
    return out.str();
}

double styleMultiplier(const std::string& style)
{
    if (style == "budget") return 0.75;
    if (style == "luxury") return 1.45;
    if (style == "adventure") return 1.1;
    return 1.0;
}

http::Json allocationFor(const std::string& style, double budget)
{
    double stay = 0.35, food = 0.25, transport = 0.15, activities = 0.18, misc = 0.07;
    if (style == "budget")
    {
        stay = 0.25;
        food = 0.30;
        transport = 0.18;
        activities = 0.18;
        misc = 0.09;
    }
    else if (style == "luxury")
    {
        stay = 0.48;
        food = 0.22;
        transport = 0.12;
        activities = 0.12;
        misc = 0.06;
    }
    else if (style == "adventure")
    {
        stay = 0.28;
        food = 0.20;
        transport = 0.17;
        activities = 0.28;
        misc = 0.07;
    }
    auto round2 = [](double v) { return std::round(v * 100.0) / 100.0; };
    return {
        {"stay", round2(budget * stay)},
        {"food", round2(budget * food)},
        {"transport", round2(budget * transport)},
        {"activities", round2(budget * activities)},
        {"miscellaneous", round2(budget * misc)},
        {"currency", "INR"},
        {"estimated", true},
    };
}

struct CatalogCity
{
    std::vector<std::string> keys;
    std::string display;
    std::vector<PlannerPlace> places;
};

const std::vector<CatalogCity>& catalog()
{
    static const std::vector<CatalogCity> cities = {
        {{"paris", "france"},
         "Paris",
         {
             {"Eiffel Tower", "Evening lights and city views from the Champ de Mars.", "sightseeing", "17:30", 3300, "Metro line 6 or 9"},
             {"Louvre Museum", "Timed-entry galleries; start with the Denon wing.", "museums", "09:30", 2000, "Metro Palais Royal"},
             {"Le Marais food walk", "Falafel, cheese shops and patisseries.", "food", "13:00", 4200, "Walk"},
             {"Palace of Versailles", "Palace, gardens and Trianon estate.", "historical_places", "10:00", 3000, "RER C"},
             {"Montmartre & Sacre-Coeur", "Hilltop basilica and artists' square.", "culture", "08:30", 0, "Metro Abbesses"},
             {"Seine river cruise", "One-hour cruise from Port de la Bourdonnais.", "relaxation", "19:00", 1600, "Walk from Trocadero"},
             {"Latin Quarter bookshops", "Shakespeare and Company and cafe-hopping.", "family", "16:00", 800, "Walk"},
             {"Galeries Lafayette", "Haussmann flagship and rooftop lookout.", "shopping", "15:00", 2500, "Metro Chaussee d'Antin"},
         }},
        {{"tokyo", "japan"},
         "Tokyo",
         {
             {"Senso-ji Temple", "Tokyo's oldest temple; arrive before the crowds.", "culture", "07:30", 0, "Ginza line to Asakusa"},
             {"Tokyo Skytree", "Tembo deck city panorama.", "photography", "11:00", 2100, "Hanzomon line"},
             {"Shibuya Crossing", "Watch from the station footbridge.", "sightseeing", "16:00", 0, "JR Yamanote"},
             {"Takeshita Street", "Harajuku street fashion and crepes.", "shopping", "14:00", 3000, "JR Harajuku"},
             {"Tsukiji Outer Market", "Sushi breakfast and tamagoyaki stalls.", "food", "08:00", 6000, "Oedo line"},
             {"Meiji Jingu", "Forested shrine next to Harajuku.", "nature", "09:00", 0, "JR Harajuku"},
             {"Omoide Yokocho", "Yakitori alley near Shinjuku station.", "nightlife", "19:30", 4500, "JR Shinjuku"},
             {"teamLab Planets", "Immersive digital art (book ahead).", "family", "13:00", 3800, "Yurikamome"},
         }},
        {{"goa"},
         "Goa",
         {
             {"Baga Beach", "Busy north Goa beach with water sports.", "beaches", "11:00", 0, "Scooter"},
             {"Tito's Lane", "Clubs and bars off Baga beach road.", "nightlife", "21:00", 3200, "Walk"},
             {"Basilica of Bom Jesus", "UNESCO-listed 16th-century church.", "historical_places", "10:00", 0, "Taxi"},
             {"Fontainhas", "Portuguese-style lanes in Panaji.", "culture", "16:00", 500, "Walk"},
             {"Palolem Beach", "Calm crescent beach in South Goa.", "beaches", "12:00", 0, "Scooter"},
             {"Spice plantation lunch", "Guided walk and Goan thali.", "food", "13:00", 1800, "Taxi"},
             {"Dudhsagar viewpoint", "Shared jeep to the waterfall (seasonal).", "adventure", "08:00", 2500, "Jeep"},
             {"Anjuna flea stalls", "Clothes, jewellery and souvenirs.", "shopping", "17:00", 1500, "Scooter"},
         }},
        {{"jaipur", "rajasthan", "pink city"},
         "Jaipur",
         {
             {"Amber Fort", "Hilltop fort; hire a licensed guide.", "historical_places", "09:00", 1200, "Taxi or bus"},
             {"Panna Meena ka Kund", "Symmetrical stepwell near Amber.", "photography", "11:30", 0, "Walk"},
             {"City Palace", "Royal residence and museum.", "museums", "10:00", 700, "Auto"},
             {"Hawa Mahal", "Palace of Winds facade.", "sightseeing", "08:30", 200, "Walk"},
             {"Johari Bazaar", "Jewellery and block-print textiles.", "shopping", "16:00", 5200, "Walk"},
             {"Chokhi Dhani dinner", "Village-style Rajasthani meal and folk shows.", "food", "19:00", 1200, "Taxi"},
             {"Nahargarh sunset", "Fort walls overlooking the city.", "nature", "17:30", 100, "Taxi"},
             {"Jantar Mantar", "18th-century astronomical instruments.", "family", "12:00", 50, "Walk"},
         }},
        {{"manali", "himachal"},
         "Manali",
         {
             {"Hadimba Temple", "Wooden temple in a cedar forest.", "culture", "10:00", 0, "Walk"},
             {"Old Manali cafes", "Riverside cafes and live music.", "food", "13:00", 800, "Walk"},
             {"Solang Valley paragliding", "Tandem flight, 10-15 minutes.", "adventure", "10:30", 3500, "Taxi"},
             {"Solang ropeway", "Cable car with valley views.", "photography", "12:30", 800, "Walk"},
             {"Atal Tunnel to Sissu", "Drive under Rohtang to the waterfall.", "nature", "09:00", 1500, "Taxi"},
             {"Beas rafting", "Grade II-III rapids near Kullu.", "adventure", "09:30", 1500, "Taxi"},
             {"Mall Road", "Shops and snacks before evening.", "shopping", "18:00", 1000, "Walk"},
             {"Vashisht hot springs", "Village baths and temples.", "relaxation", "16:00", 0, "Walk"},
         }},
        {{"kerala", "kochi", "alleppey", "kochi"},
         "Kerala",
         {
             {"Chinese fishing nets", "Cantilevered nets on the Fort Kochi shore.", "culture", "17:30", 0, "Walk"},
             {"Kathakali performance", "Evening show; watch the make-up first.", "culture", "18:30", 500, "Walk"},
             {"Alleppey houseboat", "Backwater cruise with meals included.", "nature", "12:00", 9000, "Taxi to jetty"},
             {"Karimeen lunch", "Pearl-spot fish at a lakeside shack.", "food", "13:30", 600, "Boat"},
             {"Marari Beach", "Quiet fishing-village beach.", "beaches", "10:00", 0, "Taxi"},
             {"Mattancherry spice market", "Pepper, cardamom and tea.", "shopping", "16:00", 1500, "Auto"},
             {"Jew Town galleries", "Antique shops and the synagogue street.", "historical_places", "11:00", 200, "Walk"},
             {"Tea museum Munnar", "Estate walk if you add a highland day.", "family", "14:00", 400, "Taxi"},
         }},
        {{"bali", "indonesia"},
         "Bali",
         {
             {"Ubud rice terraces", "Tegalalang viewpoint and short walk.", "nature", "08:00", 300, "Scooter or driver"},
             {"Tirta Empul", "Temple water purification (sarong required).", "culture", "10:00", 500, "Driver"},
             {"Ubud Palace & market", "Evening market and palace courtyard.", "shopping", "17:00", 1200, "Walk"},
             {"Nusa Penida day boat", "Cliffs and beaches (weather dependent).", "adventure", "07:00", 4500, "Sanur harbour"},
             {"Jimbaran seafood", "Grilled fish on the beach at sunset.", "food", "18:30", 2500, "Taxi"},
             {"Seminyak beach club", "Late afternoon by the Indian Ocean.", "beaches", "15:00", 1800, "Taxi"},
             {"Lempuyang gates", "Famous temple gates (start very early).", "photography", "05:30", 800, "Driver"},
             {"Uluwatu kecak dance", "Sunset dance at the clifftop temple.", "family", "17:45", 1500, "Taxi"},
         }},
        {{"dubai", "uae"},
         "Dubai",
         {
             {"Burj Khalifa at the Top", "Timed observation deck tickets.", "sightseeing", "18:00", 4500, "Metro Burj Khalifa"},
             {"Dubai Mall fountain", "Choreographed fountain show.", "family", "19:30", 0, "Walk"},
             {"Old Dubai souks", "Gold and spice souks plus abra crossing.", "shopping", "10:00", 800, "Metro Al Ras"},
             {"Desert safari", "Dune bashing, dinner and shows.", "adventure", "15:30", 5500, "Hotel pickup"},
             {"Jumeirah Beach", "Public beach near the Burj Al Arab view.", "beaches", "09:00", 0, "Taxi"},
             {"Museum of the Future", "Exhibition on tomorrow's cities.", "museums", "11:00", 3200, "Metro Emirates Towers"},
             {"Global Village", "Seasonal cultural pavilion and snacks.", "food", "17:00", 2000, "Taxi"},
             {"Dubai Marina walk", "Evening promenade and marina lights.", "relaxation", "20:00", 0, "Tram"},
         }},
        {{"ooty", "udhagamandalam"},
         "Ooty",
         {
             {"Doddabetta Peak", "Highest point in the Nilgiris; go early before the mist rolls in.", "nature", "08:30", 100, "Taxi"},
             {"Nilgiri Mountain Railway", "Heritage toy train through tea country.", "sightseeing", "09:15", 400, "Walk from Ooty station"},
             {"Government Botanical Garden", "Terraced gardens and a fossil tree trunk.", "nature", "10:30", 50, "Walk or auto"},
             {"Tea factory and museum", "Tea tasting and a short factory tour.", "culture", "11:30", 150, "Taxi"},
             {"Pykara Lake and Falls", "Quiet lake and waterfall on the Mudumalai road.", "nature", "10:00", 400, "Taxi"},
             {"Ooty Lake boating", "Paddle or row boats on the lake.", "relaxation", "15:00", 400, "Auto"},
             {"Rose Garden", "Thousands of rose varieties on terraced slopes.", "nature", "16:00", 50, "Walk or auto"},
             {"Varkey and chocolate tasting", "Local biscuits and homemade chocolates.", "food", "13:00", 300, "Walk"},
             {"Charing Cross and Ooty market", "Spices, woollens and chocolates.", "shopping", "17:00", 1000, "Walk"},
         }},
        {{"kodaikanal", "kodai"},
         "Kodaikanal",
         {
             {"Coaker's Walk", "Cliffside path with valley views.", "sightseeing", "07:30", 30, "Walk"},
             {"Kodai Lake", "Star-shaped lake; walk the loop or take a boat.", "relaxation", "09:30", 400, "Walk"},
             {"Pillar Rocks", "Three granite pillars rising about 120 m.", "photography", "11:00", 30, "Taxi"},
             {"Guna Caves", "Rocky cave area in the Pine Forest; follow the marked paths.", "adventure", "12:00", 30, "Taxi"},
             {"Pine Forest", "Quiet pine trails and viewpoints.", "nature", "10:00", 30, "Taxi"},
             {"Berijam Lake", "Forest-lined lake; entry needs a forest department permit.", "adventure", "09:00", 500, "Taxi"},
             {"Bryant Park", "Botanical garden beside the lake.", "nature", "15:30", 30, "Walk"},
             {"Local meals near the lake", "Simple South Indian lunch and hot tea.", "food", "13:00", 500, "Walk"},
             {"Chocolates and bazaar", "Homemade chocolates, spices and woollens.", "shopping", "17:00", 800, "Walk"},
         }},
        {{"chennai", "madras"},
         "Chennai",
         {
             {"Marina Beach", "Long city beach; best at sunrise or after sunset.", "beaches", "06:30", 0, "Taxi or metro"},
             {"Kapaleeshwarar Temple", "Dravidian temple in Mylapore with a colourful gopuram.", "culture", "08:00", 0, "Auto"},
             {"Mylapore food walk", "Filter coffee, idli, dosa and sweets.", "food", "08:30", 600, "Walk"},
             {"Mahabalipuram Shore Temple", "UNESCO World Heritage site, about an hour from the city.", "historical_places", "09:00", 100, "Taxi"},
             {"San Thome Basilica", "Neo-Gothic church near the shore.", "historical_places", "10:00", 0, "Auto"},
             {"Fort St. George museum", "Colonial-era fort and museum.", "museums", "10:30", 50, "Taxi"},
             {"Government Museum, Egmore", "Bronzes, archaeology and natural history galleries.", "museums", "11:30", 50, "Metro Egmore"},
             {"DakshinaChitra", "Heritage village of South Indian arts and crafts.", "family", "11:00", 300, "Taxi"},
             {"Pondy Bazaar, T. Nagar", "Textiles, jewellery and street shopping.", "shopping", "17:00", 2000, "Metro"},
         }},
        {{"delhi"},
         "Delhi",
         {
             {"Red Fort", "Mughal fort; closed on Mondays.", "historical_places", "09:30", 300, "Metro Lal Qila"},
             {"Humayun's Tomb", "Garden tomb, a forerunner of the Taj Mahal.", "historical_places", "10:00", 300, "Metro"},
             {"Qutub Minar", "13th-century victory tower complex.", "historical_places", "09:00", 300, "Metro Qutab Minar"},
             {"Chandni Chowk food walk", "Parathas, chaat and old-city lanes.", "food", "11:30", 800, "Metro Chandni Chowk"},
             {"Lodhi Garden", "Tombs among lawns; good for a slow morning.", "nature", "07:30", 0, "Taxi"},
             {"Lotus Temple", "Bahai House of Worship; free entry.", "culture", "16:00", 0, "Metro Kalkaji Mandir"},
             {"India Gate", "War memorial and lawns; lovely at dusk.", "sightseeing", "18:00", 0, "Metro"},
             {"Dilli Haat", "Craft stalls and regional food courts.", "shopping", "15:00", 1500, "Metro INA"},
         }},
    };
    return cities;
}

std::vector<PlannerPlace> genericPlaces(const std::string& destination)
{
    return {
        {"Old town walk in " + destination, "Orient yourself with a self-guided historic core walk.", "sightseeing", "09:00", 0, "Walk"},
        {"Local food market", "Try regional breakfast dishes and produce.", "food", "08:30", 1200, "Walk or transit"},
        {"City museum", "Context for the destination's history.", "museums", "11:00", 800, "Transit"},
        {"Viewpoint or park", "Golden-hour photos and a slower afternoon.", "nature", "16:30", 0, "Walk"},
        {"Craft or bazaar street", "Independent shops and souvenirs.", "shopping", "15:00", 1500, "Walk"},
        {"Evening neighbourhood", "Dinner and a night stroll.", "nightlife", "19:30", 1800, "Taxi"},
        {"Day trip lookout", "Short trip to a nearby natural or heritage site.", "adventure", "08:00", 2500, "Bus or hire car"},
        {"Family garden or waterfront", "Unhurried outdoor time.", "family", "14:00", 400, "Walk"},
        {"Heritage monument", "Key landmark with a licensed guide if available.", "historical_places", "10:00", 900, "Transit"},
        {"Wellness hour", "Spa, beach or quiet cafe — match the local setting.", "relaxation", "17:00", 1500, "Walk"},
    };
}

bool matchesInterest(const PlannerPlace& place, const std::vector<std::string>& interests)
{
    if (interests.empty())
    {
        return true;
    }
    for (const auto& interest : interests)
    {
        if (place.category == interest) return true;
        if (interest == "historical_places" && place.category == "history") return true;
        if (interest == "history" && place.category == "historical_places") return true;
        if (interest == "culture" && (place.category == "museums" || place.category == "historical_places")) return true;
        if (interest == "nature" && place.category == "beaches") return true;
        if (interest == "beaches" && place.category == "nature") return true;
        if (interest == "photography") return true;
    }
    return false;
}

const CatalogCity* findCity(const std::string& destination)
{
    const auto needle = lowerCopy(destination);
    for (const auto& city : catalog())
    {
        for (const auto& key : city.keys)
        {
            if (needle.find(key) != std::string::npos)
            {
                return &city;
            }
        }
    }
    return nullptr;
}

std::string mealFor(const std::string& dietary, const std::string& style, bool dayOne)
{
    std::string base = dayOne ? "Welcome dinner featuring regional specials"
                              : "Breakfast at the stay, a local lunch, and a sit-down dinner";
    if (!dietary.empty())
    {
        base += ". Dietary note: " + dietary;
    }
    if (style == "luxury")
    {
        base += ". Prioritise highly rated restaurants.";
    }
    else if (style == "budget")
    {
        base += ". Favour markets, bakeries and set lunches.";
    }
    return base;
}

std::string stayFor(const PlannerInput& input)
{
    if (!input.accommodationPreference.empty())
    {
        return "Stay preference: " + input.accommodationPreference;
    }
    if (input.travelStyle == "luxury") return "Upscale hotel or resort close to the main sights";
    if (input.travelStyle == "budget") return "Guesthouse, hostel private room, or mid-range homestay";
    if (input.travelStyle == "adventure") return "Lodge or camp with early activity access";
    return "Well-reviewed mid-range hotel or serviced apartment";
}

std::string transportFor(const PlannerInput& input)
{
    if (!input.transportationPreference.empty())
    {
        return input.transportationPreference;
    }
    if (input.travelStyle == "luxury") return "Private transfers and licensed taxis";
    if (input.travelStyle == "budget") return "Public transit, walking and occasional rideshares";
    return "Mix of transit, walking and a few taxis";
}

std::string titleFor(int day, int duration, const std::string& city, const std::vector<PlannerPlace>& picks)
{
    if (day == 1) return "Arrival and first look at " + city;
    if (day == duration) return "Last highlights and departure";
    if (!picks.empty()) return picks.front().name + " and nearby " + city;
    return "Exploring " + city;
}
} // namespace

PlannerResult ItineraryPlanner::generate(const PlannerInput& input) const
{
    PlannerResult result;
    result.budgetAllocation = allocationFor(input.travelStyle, input.budget);
    result.source = "rule-based-catalog";

    const auto* city = findCity(input.destination);
    auto pool = city ? city->places : genericPlaces(input.destination);
    const std::string label = city ? city->display : input.destination;
    if (!city)
    {
        result.notes = "No curated dataset matched this destination. The planner used a generic template "
                       "shaped by your interests, duration and budget. Treat place names as starting points, not live listings.";
    }
    else
    {
        result.notes = "Places are drawn from a curated offline catalog for " + label
            + ". Costs are estimates in the trip currency, scaled for travel style and group size — not live quotes.";
    }

    std::vector<PlannerPlace> preferred;
    std::vector<PlannerPlace> rest;
    for (const auto& place : pool)
    {
        if (matchesInterest(place, input.interests)) preferred.push_back(place);
        else rest.push_back(place);
    }
    preferred.insert(preferred.end(), rest.begin(), rest.end());
    if (preferred.empty())
    {
        preferred = genericPlaces(input.destination);
    }

    const double multiplier = styleMultiplier(input.travelStyle) * std::max(1, input.members) / 1.0;
    const int duration = std::max(1, std::min(input.durationDays, 60));
    const int perDay = preferred.size() >= 3 ? 3 : std::max(1, static_cast<int>(preferred.size()));

    double running = 0;
    for (int day = 1; day <= duration; ++day)
    {
        PlannerDay planned;
        planned.dayNumber = day;
        planned.date = input.startDate.empty() ? "" : addDays(input.startDate, day - 1);
        planned.meals = mealFor(input.dietaryPreferences, input.travelStyle, day == 1);
        planned.accommodation = stayFor(input);
        planned.transportation = transportFor(input);

        std::vector<PlannerPlace> picks;
        for (int i = 0; i < perDay; ++i)
        {
            auto place = preferred[static_cast<std::size_t>((day - 1) * perDay + i) % preferred.size()];
            place.baseCost = std::round(place.baseCost * multiplier * 100.0) / 100.0;
            if (day == duration && i == perDay - 1)
            {
                place.name = "Buffer time / airport or station transfer";
                place.description = "Leave unscheduled time for checkout and travel home.";
                place.category = "other";
                place.baseCost = std::round(800 * multiplier * 100.0) / 100.0;
                place.visitTime = "15:00";
            }
            picks.push_back(place);
            planned.dailyBudget += place.baseCost;
        }
        planned.places = picks;
        planned.title = titleFor(day, duration, label, picks);
        planned.description = "Day " + std::to_string(day) + " in " + label
            + " mixes your selected interests with estimated costs.";
        running += planned.dailyBudget;
        result.days.push_back(std::move(planned));
    }

    // Scale activity costs toward the activities slice of the budget when possible.
    const double activitiesBudget = result.budgetAllocation.value("activities", 0.0);
    if (running > 0 && activitiesBudget > 0)
    {
        const double scale = std::min(1.6, std::max(0.45, activitiesBudget / running));
        running = 0;
        for (auto& day : result.days)
        {
            day.dailyBudget = 0;
            for (auto& place : day.places)
            {
                place.baseCost = std::round(place.baseCost * scale * 100.0) / 100.0;
                day.dailyBudget += place.baseCost;
            }
            running += day.dailyBudget;
        }
    }

    result.estimatedTotal = std::round(running * 100.0) / 100.0;
    return result;
}
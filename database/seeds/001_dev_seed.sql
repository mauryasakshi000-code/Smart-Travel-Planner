-- Development seed data. Rows are linked by natural keys (user email, trip
-- title, blog title) instead of hard-coded ids. All money is in INR.
--
-- Seed accounts have a placeholder password_hash that matches no password,
-- so they cannot be used to log in.
--
-- Deliberate gaps for practising joins: Kabir Singh has no trips, blogs or
-- likes; two blogs are unpublished drafts; the Goa trip is over budget.

-- ---------------------------------------------------------------- users
INSERT INTO users (full_name, email, password_hash, created_at, updated_at)
SELECT v.full_name, v.email, 'seed-account-no-login', v.created_at::timestamptz, v.created_at::timestamptz
FROM (VALUES
    ('Aarav Sharma', 'aarav.sharma@example.com', '2025-10-02 10:15:00+05:30'),
    ('Priya Patel',  'priya.patel@example.com',  '2025-10-05 18:40:00+05:30'),
    ('Rohan Verma',  'rohan.verma@example.com',  '2025-11-20 09:05:00+05:30'),
    ('Ananya Iyer',  'ananya.iyer@example.com',  '2025-12-11 21:30:00+05:30'),
    ('Kabir Singh',  'kabir.singh@example.com',  '2026-06-01 12:00:00+05:30')
) AS v(full_name, email, created_at);

-- ---------------------------------------------------------------- sessions (all expired)
INSERT INTO sessions (user_id, session_token, expires_at, created_at)
SELECT u.id, md5(random()::text) || md5(random()::text), v.expires_at::timestamptz, v.created_at::timestamptz
FROM (VALUES
    ('aarav.sharma@example.com', '2026-04-15 20:00:00+05:30', '2026-04-22 20:00:00+05:30'),
    ('priya.patel@example.com',  '2026-04-03 19:30:00+05:30', '2026-04-10 19:30:00+05:30'),
    ('rohan.verma@example.com',  '2026-05-26 18:00:00+05:30', '2026-06-02 18:00:00+05:30')
) AS v(email, created_at, expires_at)
JOIN users u ON u.email = v.email;

-- ---------------------------------------------------------------- trips
INSERT INTO trips (user_id, title, destination, budget, duration_days, interests, created_at, updated_at)
SELECT u.id, v.title, v.destination, v.budget, v.duration_days, v.interests,
       v.created_at::timestamptz, v.created_at::timestamptz
FROM (VALUES
    ('aarav.sharma@example.com', 'Paris Food & Culture Getaway', 'Paris, France', 150000.00, 4,
        ARRAY['food', 'culture', 'history'],                  '2026-02-14 20:00:00+05:30'),
    ('aarav.sharma@example.com', 'Goa Beach Weekend',            'Goa, India',     25000.00, 3,
        ARRAY['nature', 'nightlife', 'food'],                 '2025-12-01 19:30:00+05:30'),
    ('priya.patel@example.com',  'Tokyo Cherry Blossom Trip',    'Tokyo, Japan',  200000.00, 4,
        ARRAY['culture', 'food', 'shopping', 'photography'],  '2026-01-22 22:10:00+05:30'),
    ('priya.patel@example.com',  'Jaipur Heritage Trail',        'Jaipur, India',  30000.00, 3,
        ARRAY['history', 'culture', 'shopping'],              '2025-10-20 11:00:00+05:30'),
    ('rohan.verma@example.com',  'Manali Adventure Escape',      'Manali, India',  35000.00, 4,
        ARRAY['adventure', 'nature', 'photography'],          '2026-04-02 08:45:00+05:30'),
    ('ananya.iyer@example.com',  'Kerala Backwaters Retreat',    'Kochi, India',   40000.00, 3,
        ARRAY['nature', 'culture', 'food'],                   '2025-12-15 16:20:00+05:30')
) AS v(email, title, destination, budget, duration_days, interests, created_at)
JOIN users u ON u.email = v.email;

-- ---------------------------------------------------------------- itinerary_days
INSERT INTO itinerary_days (trip_id, day_number, date, title, description)
SELECT t.id, v.day_number, v.day_date::date, v.title, v.description
FROM (VALUES
    ('Paris Food & Culture Getaway', 1, '2026-04-10', 'Arrival and the Eiffel Tower',
        'Check in, then an evening at the Eiffel Tower and a Seine river cruise.'),
    ('Paris Food & Culture Getaway', 2, '2026-04-11', 'The Louvre and Le Marais',
        'Morning at the Louvre, afternoon food walk through Le Marais.'),
    ('Paris Food & Culture Getaway', 3, '2026-04-12', 'Day trip to Versailles',
        'Train to Versailles for the palace and its gardens.'),
    ('Paris Food & Culture Getaway', 4, '2026-04-13', 'Montmartre and departure',
        'Sacre-Coeur at sunrise, cafe lunch in Montmartre, evening flight.'),

    ('Goa Beach Weekend', 1, '2025-12-26', 'North Goa beaches',
        'Beach afternoon at Baga, nightlife on Tito''s Lane.'),
    ('Goa Beach Weekend', 2, '2025-12-27', 'Old Goa and Panaji',
        'Portuguese-era churches, then the Fontainhas Latin Quarter.'),
    ('Goa Beach Weekend', 3, '2025-12-28', 'South Goa and seafood',
        'Scooter ride to Palolem, seafood dinner on the way back.'),

    ('Tokyo Cherry Blossom Trip', 1, '2026-03-28', 'Asakusa and Skytree',
        'Senso-ji temple in the morning, city views from Tokyo Skytree.'),
    ('Tokyo Cherry Blossom Trip', 2, '2026-03-29', 'Shibuya and Harajuku',
        'Shibuya Crossing and street fashion on Takeshita Street.'),
    ('Tokyo Cherry Blossom Trip', 3, '2026-03-30', 'Markets and Ginza',
        'Breakfast at Tsukiji Outer Market, shopping in Ginza.'),
    ('Tokyo Cherry Blossom Trip', 4, '2026-03-31', 'Meiji Shrine and departure',
        'Quiet morning at Meiji Jingu, farewell dinner in Omoide Yokocho.'),

    ('Jaipur Heritage Trail', 1, '2025-11-14', 'Amber Fort',
        'Amber Fort with a guide, then the Panna Meena stepwell.'),
    ('Jaipur Heritage Trail', 2, '2025-11-15', 'The Pink City',
        'City Palace museum and the Hawa Mahal facade.'),
    ('Jaipur Heritage Trail', 3, '2025-11-16', 'Bazaars and a village dinner',
        'Textile shopping in Johari Bazaar, Rajasthani dinner at Chokhi Dhani.'),

    ('Manali Adventure Escape', 1, '2026-05-20', 'Arrival and Old Manali',
        'Hadimba Temple and an evening in the Old Manali cafes.'),
    ('Manali Adventure Escape', 2, '2026-05-21', 'Solang Valley',
        'Paragliding and the Solang ropeway.'),
    ('Manali Adventure Escape', 3, '2026-05-22', 'Atal Tunnel and Sissu',
        'Drive through the Atal Tunnel to the Sissu waterfall.'),
    ('Manali Adventure Escape', 4, '2026-05-23', 'Rafting and departure',
        'Morning rafting on the Beas, Mall Road before the night bus.'),

    ('Kerala Backwaters Retreat', 1, '2026-01-18', 'Fort Kochi',
        'Chinese fishing nets at sunset and a Kathakali performance.'),
    ('Kerala Backwaters Retreat', 2, '2026-01-19', 'Alleppey houseboat',
        'Overnight houseboat cruise through the backwaters.'),
    ('Kerala Backwaters Retreat', 3, '2026-01-20', 'Marari Beach and spices',
        'Slow morning at Marari Beach, spice shopping in Mattancherry.')
) AS v(trip_title, day_number, day_date, title, description)
JOIN trips t ON t.title = v.trip_title;

-- ---------------------------------------------------------------- itinerary_locations
INSERT INTO itinerary_locations (itinerary_day_id, name, description, category, estimated_cost, latitude, longitude)
SELECT d.id, v.name, v.description, v.category, v.estimated_cost, v.latitude, v.longitude
FROM (VALUES
    ('Paris Food & Culture Getaway', 1, 'Eiffel Tower', 'Summit tickets for the evening slot.', 'sightseeing', 3300.00, 48.858370, 2.294481),
    ('Paris Food & Culture Getaway', 1, 'Seine River Cruise', 'One-hour cruise from Port de la Bourdonnais.', 'sightseeing', 1600.00, 48.859500, 2.293400),
    ('Paris Food & Culture Getaway', 2, 'Louvre Museum', 'Book a timed entry; start with the Denon wing.', 'culture', 2000.00, 48.860611, 2.337644),
    ('Paris Food & Culture Getaway', 2, 'Le Marais Food Walk', 'Falafel, cheese shops and patisseries.', 'food', 4200.00, 48.855573, 2.365492),
    ('Paris Food & Culture Getaway', 3, 'Palace of Versailles', 'Passport ticket covers the palace and Trianon.', 'history', 3000.00, 48.804865, 2.120355),
    ('Paris Food & Culture Getaway', 3, 'Gardens of Versailles', 'Free on non-fountain-show days.', 'nature', 0.00, 48.807900, 2.113800),
    ('Paris Food & Culture Getaway', 4, 'Sacre-Coeur Basilica', 'Free entry; best views at sunrise.', 'culture', 0.00, 48.886705, 2.343104),
    ('Paris Food & Culture Getaway', 4, 'Montmartre Cafes', 'Long lunch around Place du Tertre.', 'food', 1800.00, 48.886400, 2.340800),

    ('Goa Beach Weekend', 1, 'Baga Beach', 'Busy beach with water sports.', 'nature', 0.00, 15.555900, 73.751600),
    ('Goa Beach Weekend', 1, 'Tito''s Lane', 'Clubs and bars off Baga beach road.', 'nightlife', 3200.00, 15.554900, 73.753800),
    ('Goa Beach Weekend', 2, 'Basilica of Bom Jesus', 'UNESCO-listed 16th-century church.', 'history', 0.00, 15.500900, 73.911600),
    ('Goa Beach Weekend', 2, 'Fontainhas Latin Quarter', 'Colourful Portuguese-style lanes in Panaji.', 'culture', 500.00, 15.497900, 73.832200),
    ('Goa Beach Weekend', 3, 'Palolem Beach', 'Calm crescent beach in South Goa.', 'nature', 0.00, 15.009900, 74.023200),
    ('Goa Beach Weekend', 3, 'Martin''s Corner', 'Goan seafood restaurant in Betalbatim.', 'food', 2400.00, 15.296600, 73.916600),

    ('Tokyo Cherry Blossom Trip', 1, 'Senso-ji Temple', 'Tokyo''s oldest temple; arrive before 8 am.', 'culture', 0.00, 35.714765, 139.796655),
    ('Tokyo Cherry Blossom Trip', 1, 'Tokyo Skytree', 'Tembo deck at 350 m.', 'photography', 2100.00, 35.710063, 139.810700),
    ('Tokyo Cherry Blossom Trip', 2, 'Shibuya Crossing', 'Watch from the station footbridge.', 'photography', 0.00, 35.659500, 139.700500),
    ('Tokyo Cherry Blossom Trip', 2, 'Takeshita Street', 'Street fashion and crepes in Harajuku.', 'shopping', 3000.00, 35.671500, 139.704800),
    ('Tokyo Cherry Blossom Trip', 3, 'Tsukiji Outer Market', 'Sushi breakfast and tamagoyaki stalls.', 'food', 6000.00, 35.665500, 139.770400),
    ('Tokyo Cherry Blossom Trip', 3, 'Ginza', 'Department stores and flagship shops.', 'shopping', 15000.00, 35.671700, 139.765000),
    ('Tokyo Cherry Blossom Trip', 4, 'Meiji Jingu', 'Forested shrine next to Harajuku station.', 'culture', 0.00, 35.676400, 139.699300),
    ('Tokyo Cherry Blossom Trip', 4, 'Omoide Yokocho', 'Tiny yakitori bars near Shinjuku station.', 'food', 4500.00, 35.693100, 139.699200),

    ('Jaipur Heritage Trail', 1, 'Amber Fort', 'Hilltop fort; hire a licensed guide at the gate.', 'history', 1200.00, 26.985500, 75.851300),
    ('Jaipur Heritage Trail', 1, 'Panna Meena ka Kund', 'Symmetrical stepwell near Amber Fort.', 'photography', 0.00, 26.987900, 75.855200),
    ('Jaipur Heritage Trail', 2, 'City Palace', 'Royal residence and museum.', 'history', 700.00, 26.925800, 75.823700),
    ('Jaipur Heritage Trail', 2, 'Hawa Mahal', 'Palace of Winds; best photographed from the cafes opposite.', 'culture', 200.00, 26.923900, 75.826700),
    ('Jaipur Heritage Trail', 3, 'Johari Bazaar', 'Jewellery and block-print textiles.', 'shopping', 5200.00, 26.921800, 75.826400),
    ('Jaipur Heritage Trail', 3, 'Chokhi Dhani', 'Village-style Rajasthani dinner and folk shows.', 'food', 1200.00, 26.766800, 75.836400),

    ('Manali Adventure Escape', 1, 'Hadimba Temple', 'Wooden temple in a cedar forest.', 'culture', 0.00, 32.248400, 77.180200),
    ('Manali Adventure Escape', 1, 'Old Manali Cafes', 'Riverside cafes and live music.', 'food', 800.00, 32.254700, 77.183300),
    ('Manali Adventure Escape', 2, 'Solang Valley Paragliding', 'Tandem flight, 10-15 minutes.', 'adventure', 3500.00, 32.316600, 77.157700),
    ('Manali Adventure Escape', 2, 'Solang Ropeway', 'Cable car with valley views.', 'photography', 800.00, 32.315700, 77.154700),
    ('Manali Adventure Escape', 3, 'Atal Tunnel', '9 km tunnel under the Rohtang range.', 'nature', 1500.00, 32.393000, 77.148000),
    ('Manali Adventure Escape', 3, 'Sissu Waterfall', 'Waterfall in the Lahaul valley.', 'photography', 0.00, 32.484000, 77.125000),
    ('Manali Adventure Escape', 4, 'Beas River Rafting', 'Grade II-III rapids near Kullu.', 'adventure', 1500.00, 32.012000, 77.130000),
    ('Manali Adventure Escape', 4, 'Mall Road', 'Shops and snacks before the bus.', 'shopping', 1000.00, 32.243600, 77.189100),

    ('Kerala Backwaters Retreat', 1, 'Chinese Fishing Nets', 'Cantilevered nets on the Fort Kochi shore.', 'culture', 0.00, 9.968300, 76.242000),
    ('Kerala Backwaters Retreat', 1, 'Kerala Kathakali Centre', 'Evening performance; arrive early to watch make-up.', 'culture', 500.00, 9.966300, 76.243700),
    ('Kerala Backwaters Retreat', 2, 'Alleppey Houseboat', 'Overnight cruise with meals included.', 'nature', 9000.00, 9.498100, 76.338800),
    ('Kerala Backwaters Retreat', 2, 'Karimeen Lunch', 'Pearl-spot fish fry at a lakeside shack.', 'food', 600.00, 9.490000, 76.330000),
    ('Kerala Backwaters Retreat', 3, 'Marari Beach', 'Quiet fishing-village beach.', 'nature', 0.00, 9.600800, 76.298500),
    ('Kerala Backwaters Retreat', 3, 'Mattancherry Spice Market', 'Pepper, cardamom and tea.', 'shopping', 1500.00, 9.957900, 76.259600)
) AS v(trip_title, day_number, name, description, category, estimated_cost, latitude, longitude)
JOIN trips t ON t.title = v.trip_title
JOIN itinerary_days d ON d.trip_id = t.id AND d.day_number = v.day_number;

-- ---------------------------------------------------------------- trip_media
-- Public sample assets from Cloudinary's "demo" cloud, so the URLs resolve.
INSERT INTO trip_media (trip_id, cloudinary_url, public_id, resource_type, created_at)
SELECT t.id,
       format('https://res.cloudinary.com/demo/%s/upload/%s.%s',
              v.resource_type, v.public_id,
              CASE v.resource_type WHEN 'video' THEN 'mp4' ELSE 'jpg' END),
       v.public_id, v.resource_type, v.created_at::timestamptz
FROM (VALUES
    ('Paris Food & Culture Getaway', 'samples/landscapes/architecture-signs', 'image', '2026-04-10 21:30:00+05:30'),
    ('Paris Food & Culture Getaway', 'samples/food/dessert',                  'image', '2026-04-11 18:10:00+05:30'),
    ('Paris Food & Culture Getaway', 'samples/landscapes/girl-urban-view',    'image', '2026-04-13 12:45:00+05:30'),
    ('Goa Beach Weekend',            'samples/landscapes/beach-boat',         'image', '2025-12-26 17:20:00+05:30'),
    ('Goa Beach Weekend',            'samples/food/fish-vegetables',          'image', '2025-12-28 21:00:00+05:30'),
    ('Goa Beach Weekend',            'samples/sea-turtle',                    'video', '2025-12-28 11:15:00+05:30'),
    ('Tokyo Cherry Blossom Trip',    'samples/landscapes/landscape-panorama', 'image', '2026-03-28 16:40:00+05:30'),
    ('Tokyo Cherry Blossom Trip',    'samples/food/pot-mussels',              'image', '2026-03-31 20:30:00+05:30'),
    ('Jaipur Heritage Trail',        'samples/food/spices',                   'image', '2025-11-16 13:00:00+05:30'),
    ('Jaipur Heritage Trail',        'samples/bike',                          'image', '2025-11-15 10:25:00+05:30'),
    ('Manali Adventure Escape',      'samples/landscapes/nature-mountains',   'image', '2026-05-21 12:05:00+05:30'),
    ('Manali Adventure Escape',      'samples/sheep',                         'image', '2026-05-22 15:30:00+05:30'),
    ('Kerala Backwaters Retreat',    'samples/landscapes/beach-boat',         'image', '2026-01-19 17:45:00+05:30'),
    ('Kerala Backwaters Retreat',    'samples/food/spices',                   'image', '2026-01-20 16:10:00+05:30')
) AS v(trip_title, public_id, resource_type, created_at)
JOIN trips t ON t.title = v.trip_title;

-- ---------------------------------------------------------------- blogs
INSERT INTO blogs (user_id, trip_id, title, content, published, published_at, created_at, updated_at)
SELECT t.user_id, t.id, v.title, v.content,
       v.published_at IS NOT NULL, v.published_at::timestamptz, v.created_at::timestamptz,
       COALESCE(v.published_at, v.created_at)::timestamptz
FROM (VALUES
    ('Paris Food & Culture Getaway', 'Four Days of Eating My Way Through Paris',
        'Paris rewards slow eating. The Le Marais food walk was the highlight: falafel on Rue des Rosiers, aged comte and a pistachio escargot pastry. The Louvre was worth the early start, and Versailles deserves a full day, gardens included.',
        '2026-04-16 19:00:00+05:30', '2026-04-15 21:00:00+05:30'),
    ('Goa Beach Weekend', 'Goa on a Budget: A Weekend Guide',
        'Take the train, rent a scooter and skip the taxis. We covered North Goa, Old Goa and Palolem in three days. Beach shacks are cheaper than restaurants, and the Arpora Saturday night market is worth a visit. We still went over budget, mostly on the night out.',
        '2026-01-03 10:30:00+05:30', '2026-01-02 22:00:00+05:30'),
    ('Tokyo Cherry Blossom Trip', 'Chasing Cherry Blossoms in Tokyo',
        'Late March is peak sakura season. Senso-ji at 7 am was calm and beautiful; by noon it was packed. Tsukiji Outer Market is the best breakfast in the city, and Omoide Yokocho is perfect for a last night of yakitori.',
        '2026-04-05 18:15:00+05:30', '2026-04-03 20:45:00+05:30'),
    ('Jaipur Heritage Trail', 'Forts and Bazaars of the Pink City',
        'Draft: notes on Amber Fort, the stepwell and bargaining for block prints in Johari Bazaar.',
        NULL, '2025-11-22 17:00:00+05:30'),
    ('Manali Adventure Escape', 'Paragliding Over Solang Valley',
        'Fifteen minutes in the air over Solang Valley was worth the whole trip. Book with a registered operator, go early before the wind picks up, and carry a warm layer even in May. The Atal Tunnel drive to Sissu was a close second.',
        '2026-05-28 09:00:00+05:30', '2026-05-26 19:30:00+05:30'),
    ('Kerala Backwaters Retreat', 'Slow Days on the Kerala Backwaters',
        'Draft: houseboat tips, the Kathakali show and where to buy spices in Mattancherry.',
        NULL, '2026-01-25 14:10:00+05:30')
) AS v(trip_title, title, content, published_at, created_at)
JOIN trips t ON t.title = v.trip_title;

-- ---------------------------------------------------------------- blog_tags
INSERT INTO blog_tags (blog_id, tag)
SELECT b.id, v.tag
FROM (VALUES
    ('Four Days of Eating My Way Through Paris', 'paris'),
    ('Four Days of Eating My Way Through Paris', 'france'),
    ('Four Days of Eating My Way Through Paris', 'europe'),
    ('Four Days of Eating My Way Through Paris', 'food'),
    ('Four Days of Eating My Way Through Paris', 'culture'),
    ('Goa on a Budget: A Weekend Guide',         'goa'),
    ('Goa on a Budget: A Weekend Guide',         'india'),
    ('Goa on a Budget: A Weekend Guide',         'beach'),
    ('Goa on a Budget: A Weekend Guide',         'budget-travel'),
    ('Chasing Cherry Blossoms in Tokyo',         'tokyo'),
    ('Chasing Cherry Blossoms in Tokyo',         'japan'),
    ('Chasing Cherry Blossoms in Tokyo',         'cherry-blossom'),
    ('Chasing Cherry Blossoms in Tokyo',         'food'),
    ('Chasing Cherry Blossoms in Tokyo',         'photography'),
    ('Forts and Bazaars of the Pink City',       'jaipur'),
    ('Forts and Bazaars of the Pink City',       'india'),
    ('Forts and Bazaars of the Pink City',       'rajasthan'),
    ('Forts and Bazaars of the Pink City',       'history'),
    ('Paragliding Over Solang Valley',           'manali'),
    ('Paragliding Over Solang Valley',           'himalayas'),
    ('Paragliding Over Solang Valley',           'adventure'),
    ('Paragliding Over Solang Valley',           'paragliding'),
    ('Slow Days on the Kerala Backwaters',       'kerala'),
    ('Slow Days on the Kerala Backwaters',       'india'),
    ('Slow Days on the Kerala Backwaters',       'backwaters')
) AS v(blog_title, tag)
JOIN blogs b ON b.title = v.blog_title;

-- ---------------------------------------------------------------- blog_media
-- Each blog reuses all media from its trip.
INSERT INTO blog_media (blog_id, cloudinary_url, public_id, resource_type, created_at)
SELECT b.id, m.cloudinary_url, m.public_id, m.resource_type, b.created_at
FROM blogs b
JOIN trip_media m ON m.trip_id = b.trip_id;

-- ---------------------------------------------------------------- blog_likes
INSERT INTO blog_likes (blog_id, user_id, created_at)
SELECT b.id, u.id, v.created_at::timestamptz
FROM (VALUES
    ('Four Days of Eating My Way Through Paris', 'priya.patel@example.com',  '2026-04-16 20:05:00+05:30'),
    ('Four Days of Eating My Way Through Paris', 'rohan.verma@example.com',  '2026-04-17 08:30:00+05:30'),
    ('Four Days of Eating My Way Through Paris', 'ananya.iyer@example.com',  '2026-04-18 13:15:00+05:30'),
    ('Four Days of Eating My Way Through Paris', 'kabir.singh@example.com',  '2026-06-02 10:00:00+05:30'),
    ('Goa on a Budget: A Weekend Guide',         'priya.patel@example.com',  '2026-01-03 12:00:00+05:30'),
    ('Goa on a Budget: A Weekend Guide',         'rohan.verma@example.com',  '2026-01-04 09:40:00+05:30'),
    ('Chasing Cherry Blossoms in Tokyo',         'aarav.sharma@example.com', '2026-04-05 19:00:00+05:30'),
    ('Chasing Cherry Blossoms in Tokyo',         'rohan.verma@example.com',  '2026-04-06 07:50:00+05:30'),
    ('Chasing Cherry Blossoms in Tokyo',         'ananya.iyer@example.com',  '2026-04-06 22:10:00+05:30'),
    ('Paragliding Over Solang Valley',           'aarav.sharma@example.com', '2026-05-28 11:20:00+05:30'),
    ('Paragliding Over Solang Valley',           'priya.patel@example.com',  '2026-05-29 18:45:00+05:30')
) AS v(blog_title, email, created_at)
JOIN blogs b ON b.title = v.blog_title
JOIN users u ON u.email = v.email;

-- ---------------------------------------------------------------- blog_comments
INSERT INTO blog_comments (blog_id, user_id, content, created_at, updated_at)
SELECT b.id, u.id, v.content, v.created_at::timestamptz, v.created_at::timestamptz
FROM (VALUES
    ('Four Days of Eating My Way Through Paris', 'priya.patel@example.com',
        'The Le Marais food walk is going straight onto my list!', '2026-04-16 20:10:00+05:30'),
    ('Four Days of Eating My Way Through Paris', 'rohan.verma@example.com',
        'How far in advance did you book the Eiffel Tower summit tickets?', '2026-04-17 08:35:00+05:30'),
    ('Four Days of Eating My Way Through Paris', 'aarav.sharma@example.com',
        'About three weeks ahead. The evening slots sell out first.', '2026-04-17 12:00:00+05:30'),
    ('Goa on a Budget: A Weekend Guide',         'rohan.verma@example.com',
        'Great tip on the scooter rental, saved me a lot on taxis.', '2026-01-04 09:45:00+05:30'),
    ('Chasing Cherry Blossoms in Tokyo',         'ananya.iyer@example.com',
        'Those Senso-ji photos look beautiful. Was it very crowded?', '2026-04-06 22:15:00+05:30'),
    ('Chasing Cherry Blossoms in Tokyo',         'priya.patel@example.com',
        'Very, by midday! Go before 8 am for quiet photos.', '2026-04-07 08:00:00+05:30'),
    ('Chasing Cherry Blossoms in Tokyo',         'kabir.singh@example.com',
        'Adding Omoide Yokocho to my Japan plan.', '2026-06-03 21:30:00+05:30'),
    ('Paragliding Over Solang Valley',           'priya.patel@example.com',
        'Paragliding in Solang has been on my bucket list forever.', '2026-05-29 18:50:00+05:30')
) AS v(blog_title, email, content, created_at)
JOIN blogs b ON b.title = v.blog_title
JOIN users u ON u.email = v.email;

-- ---------------------------------------------------------------- expenses
INSERT INTO expenses (trip_id, user_id, category, description, amount, expense_date, created_at)
SELECT t.id, t.user_id, v.category, v.description, v.amount, v.expense_date::date,
       (v.expense_date::date + TIME '21:00') AT TIME ZONE 'Asia/Kolkata'
FROM (VALUES
    ('Paris Food & Culture Getaway', 'transport',     'Return flights Delhi-Paris',              55000.00, '2026-04-10'),
    ('Paris Food & Culture Getaway', 'accommodation', 'Hotel in Le Marais, 3 nights',            52000.00, '2026-04-10'),
    ('Paris Food & Culture Getaway', 'transport',     'Navigo weekly metro pass',                 1500.00, '2026-04-10'),
    ('Paris Food & Culture Getaway', 'activities',    'Eiffel Tower summit tickets',              3300.00, '2026-04-10'),
    ('Paris Food & Culture Getaway', 'activities',    'Louvre Museum entry',                      2000.00, '2026-04-11'),
    ('Paris Food & Culture Getaway', 'food',          'Le Marais food walking tour',              4200.00, '2026-04-11'),
    ('Paris Food & Culture Getaway', 'activities',    'Palace of Versailles passport ticket',     3000.00, '2026-04-12'),
    ('Paris Food & Culture Getaway', 'food',          'Montmartre cafe lunch',                    1800.00, '2026-04-13'),
    ('Paris Food & Culture Getaway', 'shopping',      'Perfume from Galeries Lafayette',          6500.00, '2026-04-13'),

    ('Goa Beach Weekend',            'transport',     'Train Mumbai-Madgaon, return',             4200.00, '2025-12-26'),
    ('Goa Beach Weekend',            'accommodation', 'Beach hut in Baga, 2 nights',              9000.00, '2025-12-26'),
    ('Goa Beach Weekend',            'transport',     'Scooter rental, 3 days',                   1200.00, '2025-12-26'),
    ('Goa Beach Weekend',            'activities',    'Club entry and drinks on Tito''s Lane',    3200.00, '2025-12-26'),
    ('Goa Beach Weekend',            'food',          'Beach shack meals',                        3500.00, '2025-12-27'),
    ('Goa Beach Weekend',            'shopping',      'Arpora Saturday night market',             2800.00, '2025-12-27'),
    ('Goa Beach Weekend',            'food',          'Seafood dinner at Martin''s Corner',       2400.00, '2025-12-28'),
    ('Goa Beach Weekend',            'other',         'Sunscreen and pharmacy',                    700.00, '2025-12-28'),

    ('Tokyo Cherry Blossom Trip',    'transport',     'Return flights Mumbai-Tokyo',             68000.00, '2026-03-28'),
    ('Tokyo Cherry Blossom Trip',    'accommodation', 'Shinjuku hotel, 3 nights',                60000.00, '2026-03-28'),
    ('Tokyo Cherry Blossom Trip',    'transport',     'Suica card top-up',                        3500.00, '2026-03-28'),
    ('Tokyo Cherry Blossom Trip',    'activities',    'Tokyo Skytree Tembo deck',                 2100.00, '2026-03-28'),
    ('Tokyo Cherry Blossom Trip',    'food',          'Tsukiji Outer Market breakfast',           6000.00, '2026-03-30'),
    ('Tokyo Cherry Blossom Trip',    'shopping',      'Ginza department stores',                 15000.00, '2026-03-30'),
    ('Tokyo Cherry Blossom Trip',    'food',          'Izakaya dinner in Omoide Yokocho',         4500.00, '2026-03-31'),

    ('Jaipur Heritage Trail',        'transport',     'Train Delhi-Jaipur, return',               3800.00, '2025-11-14'),
    ('Jaipur Heritage Trail',        'accommodation', 'Heritage haveli, 2 nights',                9600.00, '2025-11-14'),
    ('Jaipur Heritage Trail',        'activities',    'Amber Fort entry and guide',               1200.00, '2025-11-14'),
    ('Jaipur Heritage Trail',        'activities',    'City Palace and Hawa Mahal tickets',        900.00, '2025-11-15'),
    ('Jaipur Heritage Trail',        'transport',     'Autos and cabs',                           1500.00, '2025-11-15'),
    ('Jaipur Heritage Trail',        'shopping',      'Block-print textiles at Johari Bazaar',    5200.00, '2025-11-16'),
    ('Jaipur Heritage Trail',        'food',          'Dinner at Chokhi Dhani',                   1200.00, '2025-11-16'),

    ('Manali Adventure Escape',      'transport',     'Volvo bus Delhi-Manali, return',           4800.00, '2026-05-20'),
    ('Manali Adventure Escape',      'accommodation', 'Guesthouse in Old Manali, 3 nights',      10500.00, '2026-05-20'),
    ('Manali Adventure Escape',      'activities',    'Paragliding at Solang Valley',             3500.00, '2026-05-21'),
    ('Manali Adventure Escape',      'transport',     'Taxi to Atal Tunnel and Sissu',            4000.00, '2026-05-22'),
    ('Manali Adventure Escape',      'other',         'Snow jacket and boots rental',              900.00, '2026-05-22'),
    ('Manali Adventure Escape',      'activities',    'River rafting on the Beas',                1500.00, '2026-05-23'),
    ('Manali Adventure Escape',      'food',          'Meals and cafes',                          3200.00, '2026-05-23'),

    ('Kerala Backwaters Retreat',    'transport',     'Return flights Bengaluru-Kochi',           7500.00, '2026-01-18'),
    ('Kerala Backwaters Retreat',    'accommodation', 'Homestay in Fort Kochi, 1 night',          4500.00, '2026-01-18'),
    ('Kerala Backwaters Retreat',    'activities',    'Kathakali performance',                     500.00, '2026-01-18'),
    ('Kerala Backwaters Retreat',    'accommodation', 'Alleppey houseboat with meals, 1 night',   9000.00, '2026-01-19'),
    ('Kerala Backwaters Retreat',    'shopping',      'Spices from Mattancherry',                 1500.00, '2026-01-20')
) AS v(trip_title, category, description, amount, expense_date)
JOIN trips t ON t.title = v.trip_title;

import { useEffect, useState } from 'react';
import { Link } from 'react-router-dom';

import { fetchTrips } from '../services/tripService';
import { formatMoney } from '../utils/formatters';

function formatDate(value) {
  if (!value) return '';
  const date = new Date(`${value}T00:00:00`);
  if (Number.isNaN(date.getTime())) return value;
  return date.toLocaleDateString('en-IN', { day: 'numeric', month: 'short', year: 'numeric' });
}

function titleCase(value) {
  return value ? value.charAt(0).toUpperCase() + value.slice(1) : '';
}

export default function TripsPage() {
  const [trips, setTrips] = useState([]);
  const [error, setError] = useState('');
  const [loading, setLoading] = useState(true);

  useEffect(() => {
    let cancelled = false;
    (async () => {
      try {
        const data = await fetchTrips();
        if (!cancelled) setTrips(Array.isArray(data) ? data : data?.trips ?? []);
      } catch (err) {
        if (!cancelled) setError(err.message);
      } finally {
        if (!cancelled) setLoading(false);
      }
    })();
    return () => {
      cancelled = true;
    };
  }, []);

  if (loading) return <div className="skeleton-grid" aria-busy="true"><div className="skeleton" /><div className="skeleton" /></div>;
  if (error) return <p className="alert alert--error">{error}</p>;

  return (
    <div className="stack">
      <header className="page-header">
        <div>
          <p className="page-header__eyebrow">My Trips</p>
          <h1>Your trips</h1>
          <p className="page-header__subtitle">
            {trips.length === 0
              ? 'Nothing planned yet.'
              : `${trips.length} trip${trips.length === 1 ? '' : 's'} saved to your account.`}
          </p>
        </div>
        <Link to="/plan-trip" className="button">+ Plan a trip</Link>
      </header>

      {trips.length === 0 ? (
        <section className="card empty-state">
          <div className="empty-state__icon" aria-hidden="true">🗺️</div>
          <h2>No trips yet</h2>
          <p>Create your first trip and we'll build a day-by-day itinerary for you.</p>
          <Link to="/plan-trip" className="button">Plan your first trip</Link>
        </section>
      ) : (
        <div className="trips-grid">
          {trips.map((trip) => (
            <Link key={trip.id} to={`/trips/${trip.id}`} className="trip-card">
              <div className="trip-card__top">
                <h2>{trip.tripName}</h2>
                <span className={`trip-badge trip-badge--${trip.status}`}>{titleCase(trip.status)}</span>
              </div>
              <p className="trip-card__destination">📍 {trip.destination}</p>

              <dl className="trip-card__facts">
                <div>
                  <dt>Dates</dt>
                  <dd>
                    {trip.startDate ? `${formatDate(trip.startDate)} → ${formatDate(trip.endDate)}` : `${trip.durationDays} days`}
                  </dd>
                </div>
                <div>
                  <dt>Duration</dt>
                  <dd>{trip.durationDays} days</dd>
                </div>
                <div>
                  <dt>Travellers</dt>
                  <dd>{trip.members}</dd>
                </div>
                <div>
                  <dt>Budget</dt>
                  <dd>{formatMoney(trip.budget)}</dd>
                </div>
              </dl>

              {trip.activities?.length > 0 && (
                <div className="chip-row">
                  {trip.activities.map((activity) => (
                    <span key={activity} className="chip">{titleCase(activity)}</span>
                  ))}
                </div>
              )}
              <span className="trip-card__cta">View itinerary →</span>
            </Link>
          ))}
        </div>
      )}
    </div>
  );
}
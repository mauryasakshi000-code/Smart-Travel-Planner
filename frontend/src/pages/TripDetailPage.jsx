import { useEffect, useState } from 'react';
import { Link, useNavigate, useParams } from 'react-router-dom';

import { deleteTrip, fetchTrip, generateItinerary } from '../services/tripService';
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

export default function TripDetailPage() {
  const { id } = useParams();
  const navigate = useNavigate();
  const [trip, setTrip] = useState(null);
  const [error, setError] = useState('');
  const [loading, setLoading] = useState(true);
  const [busy, setBusy] = useState(false);

  useEffect(() => {
    let cancelled = false;
    setLoading(true);
    (async () => {
      try {
        const data = await fetchTrip(id);
        if (!cancelled) setTrip(data.trip);
      } catch (err) {
        if (!cancelled) setError(err.message);
      } finally {
        if (!cancelled) setLoading(false);
      }
    })();
    return () => {
      cancelled = true;
    };
  }, [id]);

  async function handleRegenerate() {
    setBusy(true);
    setError('');
    try {
      const data = await generateItinerary(id);
      setTrip(data.trip);
    } catch (err) {
      setError(err.message);
    } finally {
      setBusy(false);
    }
  }

  async function handleDelete() {
    if (!window.confirm('Delete this trip and its itinerary?')) return;
    setBusy(true);
    try {
      await deleteTrip(id);
      navigate('/trips');
    } catch (err) {
      setError(err.message);
      setBusy(false);
    }
  }

  if (loading) return <div className="skeleton-grid" aria-busy="true"><div className="skeleton" /><div className="skeleton" /></div>;
  if (!trip) return <p className="alert alert--error">{error || 'Trip not found.'}</p>;

  const days = trip.itinerary?.days ?? [];

  return (
    <div className="stack">
      <Link to="/trips" className="back-link">← All trips</Link>

      <section className="detail-hero">
        <div className="detail-hero__top">
          <div>
            <p className="detail-hero__eyebrow">📍 {trip.destination}</p>
            <h1>{trip.tripName}</h1>
          </div>
          <span className={`trip-badge trip-badge--${trip.status}`}>{titleCase(trip.status)}</span>
        </div>

        <div className="detail-facts">
          <div>
            <span>Dates</span>
            <strong>{trip.startDate ? `${formatDate(trip.startDate)} → ${formatDate(trip.endDate)}` : 'Not set'}</strong>
          </div>
          <div>
            <span>Duration</span>
            <strong>{trip.durationDays} days</strong>
          </div>
          <div>
            <span>Travellers</span>
            <strong>{trip.members}</strong>
          </div>
          <div>
            <span>Style</span>
            <strong>{titleCase(trip.travelStyle)}</strong>
          </div>
          <div>
            <span>Budget</span>
            <strong>{formatMoney(trip.budget)}</strong>
          </div>
        </div>
      </section>

      <div className="detail-actions">
        <button type="button" className="button" onClick={handleRegenerate} disabled={busy}>
          {busy ? 'Working...' : '↻ Regenerate itinerary'}
        </button>
        <button type="button" className="button button--danger" onClick={handleDelete} disabled={busy}>
          Delete trip
        </button>
      </div>

      {error && <p className="alert alert--error" role="alert">{error}</p>}

      <section className="stack">
        <div className="section-title">
          <h2>Itinerary</h2>
          {days.length > 0 && (
            <span className="section-title__meta">
              Estimated total {formatMoney(trip.itinerary.estimatedTotal)} of {formatMoney(trip.budget)} budget
            </span>
          )}
        </div>

        {days.length === 0 ? (
          <div className="card empty-state">
            <div className="empty-state__icon" aria-hidden="true">📝</div>
            <h2>No itinerary yet</h2>
            <p>Click "Regenerate itinerary" to build one.</p>
          </div>
        ) : (
          days.map((day) => (
            <article key={day.day} className="day-card">
              <header className="day-card__header">
                <span className="day-card__number">Day {day.day}</span>
                <div className="day-card__title">
                  <h3>{day.title}</h3>
                  {day.date && <span>{formatDate(day.date)}</span>}
                </div>
                <span className="day-card__budget">{formatMoney(day.dailyBudget)}</span>
              </header>

              {day.description && <p className="day-card__description">{day.description}</p>}

              {day.locations.length > 0 && (
                <ul className="timeline">
                  {day.locations.map((place) => (
                    <li key={`${day.day}-${place.sequenceNumber}`}>
                      <span className="timeline__time">{place.visitTime || '—'}</span>
                      <div className="timeline__body">
                        <strong>{place.name}</strong>
                        {place.description && <p>{place.description}</p>}
                        <span className="chip">{titleCase(place.category)}</span>
                      </div>
                      <span className="timeline__cost">{formatMoney(place.estimatedCost)}</span>
                    </li>
                  ))}
                </ul>
              )}

              {(day.meals || day.accommodation || day.transportation) && (
                <div className="day-card__footer">
                  {day.meals && <span>🍽️ {day.meals}</span>}
                  {day.accommodation && <span>🏨 {day.accommodation}</span>}
                  {day.transportation && <span>🚗 {day.transportation}</span>}
                </div>
              )}
            </article>
          ))
        )}

        {days.length > 0 && <p className="detail-note">All costs are estimates, not live prices.</p>}
      </section>
    </div>
  );
}
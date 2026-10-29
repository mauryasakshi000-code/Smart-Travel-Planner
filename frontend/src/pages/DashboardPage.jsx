import { useEffect, useState } from 'react';
import { Link } from 'react-router-dom';

import { useAuth } from '../context/AuthContext';
import { fetchDashboard } from '../services/dashboardService';
import { formatMoney } from '../utils/formatters';

function formatDate(value) {
  if (!value) return '';
  const date = new Date(`${value}T00:00:00`);
  if (Number.isNaN(date.getTime())) return value;
  return date.toLocaleDateString('en-IN', { day: 'numeric', month: 'short', year: 'numeric' });
}

export default function DashboardPage() {
  const { user } = useAuth();
  const [stats, setStats] = useState(null);
  const [error, setError] = useState('');
  const [loading, setLoading] = useState(true);

  useEffect(() => {
    let cancelled = false;
    (async () => {
      try {
        const data = await fetchDashboard();
        if (!cancelled) setStats(data);
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
      <section className="hero">
        <p className="hero__eyebrow">Dashboard</p>
        <h1 className="hero__title">Welcome back, {user?.fullName?.split(' ')[0] || 'traveller'}.</h1>
        <p className="hero__subtitle">Here's an overview of your trips, itineraries and budget.</p>
      </section>

      <section className="stat-grid">
        <article className="card stat"><p>Total trips</p><strong>{stats.totalTrips}</strong></article>
        <article className="card stat"><p>Upcoming</p><strong>{stats.upcomingTripsCount}</strong></article>
        <article className="card stat"><p>Saved itineraries</p><strong>{stats.savedItineraries}</strong></article>
        <article className="card stat"><p>Remaining budget</p><strong>{formatMoney(stats.remainingBudget)}</strong></article>
      </section>

      <div className="split">
        <section className="card">
          <div className="card__header">
            <h2>Recently created trips</h2>
            <Link to="/trips">View all</Link>
          </div>
          {stats.recentTrips.length === 0 ? (
            <p className="muted">No trips yet. Use "Plan Trip" in the menu to create your first itinerary.</p>
          ) : (
            <ul className="plain-list">
              {stats.recentTrips.map((trip) => (
                <li key={trip.id}>
                  <Link to={`/trips/${trip.id}`}>{trip.tripName}</Link>
                  <span className="muted">{trip.destination} · {trip.durationDays} days</span>
                </li>
              ))}
            </ul>
          )}
        </section>
        <section className="card">
          <div className="card__header">
            <h2>Upcoming trips</h2>
            <Link to="/budget">Budget tracking</Link>
          </div>
          {stats.upcomingTrips.length === 0 ? (
            <p className="muted">No dated upcoming trips.</p>
          ) : (
            <ul className="plain-list">
              {stats.upcomingTrips.map((trip) => (
                <li key={trip.id}>
                  <Link to={`/trips/${trip.id}`}>{trip.tripName}</Link>
                  <span className="muted">{formatDate(trip.startDate)} · {formatMoney(trip.budget)}</span>
                </li>
              ))}
            </ul>
          )}
        </section>
      </div>

      <section className="card">
        <div className="card__header">
          <h2>Recent expenses</h2>
          <Link to="/stories">Travel stories</Link>
        </div>
        {stats.recentExpenses.length === 0 ? (
          <p className="muted">No expenses recorded yet.</p>
        ) : (
          <ul className="plain-list">
            {stats.recentExpenses.map((item) => (
              <li key={item.id}>
                <span>{item.description || item.category}</span>
                <span className="muted">{item.tripName} · {formatMoney(item.amount)}</span>
              </li>
            ))}
          </ul>
        )}
      </section>
    </div>
  );
}
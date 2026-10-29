import { useState } from 'react';
import { useNavigate } from 'react-router-dom';

import { createTrip } from '../services/tripService';
import './TripPlannerPage.css';

// Values must match the backend's activity catalog; unknown ones are silently dropped.
const ACTIVITY_OPTIONS = [
  { value: 'nature', label: 'Nature' },
  { value: 'food', label: 'Food' },
  { value: 'adventure', label: 'Adventure' },
  { value: 'culture', label: 'Culture' },
  { value: 'history', label: 'History' },
  { value: 'relaxation', label: 'Relaxation' },
  { value: 'family', label: 'Family' },
];

const initialForm = {
  tripName: 'Weekend Mountain Trip',
  destination: 'Ooty',
  budget: '10000',
  startDate: '',
  endDate: '',
  members: '2',
  travelStyle: 'standard',
  activities: ['nature', 'food'],
};

export default function TripPlannerPage() {
  const navigate = useNavigate();
  const [form, setForm] = useState(initialForm);
  const [loading, setLoading] = useState(false);
  const [message, setMessage] = useState('');

  function handleChange(event) {
    const { name, value } = event.target;
    setForm((previous) => ({ ...previous, [name]: value }));
  }

  function toggleActivity(value) {
    setForm((previous) => ({
      ...previous,
      activities: previous.activities.includes(value)
        ? previous.activities.filter((item) => item !== value)
        : [...previous.activities, value],
    }));
  }

  async function handleSubmit(event) {
    event.preventDefault();
    setMessage('');

    if (form.endDate < form.startDate) {
      setMessage('Return date must be on or after departure date.');
      return;
    }

    setLoading(true);
    try {
      const data = await createTrip({
        tripName: form.tripName.trim(),
        destination: form.destination.trim(),
        budget: Number(form.budget),
        startDate: form.startDate,
        endDate: form.endDate,
        members: Number(form.members),
        travelStyle: form.travelStyle,
        activities: form.activities,
        currency: 'INR',
      });
      navigate(`/trips/${data.trip.id}`);
    } catch (err) {
      setMessage(`Could not create trip: ${err.message}`);
      setLoading(false);
    }
  }

  return (
    <section className="planner-page">
      <header className="planner-heading">
        <p>SMART TRAVEL PLANNER</p>
        <h1>Plan your next adventure</h1>
        <span>Enter your travel details and we'll build an itinerary.</span>
      </header>

      <form className="planner-card" onSubmit={handleSubmit}>
        <label>
          Trip name
          <input name="tripName" value={form.tripName} onChange={handleChange} required minLength={2} />
        </label>

        <label>
          Destination
          <input
            name="destination"
            value={form.destination}
            onChange={handleChange}
            placeholder="e.g. Ooty"
            required
          />
        </label>

        <div className="planner-grid">
          <label>
            Departure date
            <input type="date" name="startDate" value={form.startDate} onChange={handleChange} required />
          </label>

          <label>
            Return date
            <input
              type="date"
              name="endDate"
              value={form.endDate}
              min={form.startDate || undefined}
              onChange={handleChange}
              required
            />
          </label>

          <label>
            Budget (₹)
            <input type="number" name="budget" min="0" value={form.budget} onChange={handleChange} required />
          </label>

          <label>
            Number of travellers
            <input type="number" name="members" min="1" value={form.members} onChange={handleChange} required />
          </label>
        </div>

        <label>
          Travel style
          <select name="travelStyle" value={form.travelStyle} onChange={handleChange}>
            <option value="standard">Standard</option>
            <option value="budget">Budget</option>
            <option value="luxury">Luxury</option>
            <option value="adventure">Adventure</option>
          </select>
        </label>

        <fieldset className="planner-interests">
          <legend>Interests</legend>
          {ACTIVITY_OPTIONS.map((option) => (
            <label key={option.value}>
              <input
                type="checkbox"
                checked={form.activities.includes(option.value)}
                onChange={() => toggleActivity(option.value)}
              />
              {option.label}
            </label>
          ))}
        </fieldset>

        <button className="planner-button" disabled={loading}>
          {loading ? 'Building your itinerary...' : 'Create Trip'}
        </button>

        {message && <p role="alert" className="alert alert--error">{message}</p>}
      </form>
    </section>
  );
}
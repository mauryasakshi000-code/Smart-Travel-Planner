import { useCallback, useEffect, useMemo, useState } from 'react';
import { Link } from 'react-router-dom';

import { addExpense, deleteExpense, fetchExpenses } from '../services/expenseService';
import { fetchTrips } from '../services/tripService';
import { formatMoney } from '../utils/formatters';

// If the backend rejects a category, change these values to match its allowed list.
const CATEGORIES = ['food', 'transport', 'accommodation', 'activities', 'shopping', 'other'];

function titleCase(value) {
  return value ? value.charAt(0).toUpperCase() + value.slice(1) : '';
}

function formatDate(value) {
  if (!value) return '';
  const date = new Date(value);
  if (Number.isNaN(date.getTime())) return '';
  return date.toLocaleDateString('en-IN', { day: 'numeric', month: 'short', year: 'numeric' });
}

export default function BudgetPage() {
  const [trips, setTrips] = useState([]);
  const [tripId, setTripId] = useState('');
  const [expenses, setExpenses] = useState([]);
  const [loading, setLoading] = useState(true);
  const [expensesLoading, setExpensesLoading] = useState(false);
  const [error, setError] = useState('');
  const [saving, setSaving] = useState(false);
  const [form, setForm] = useState({ category: 'food', amount: '', description: '' });

  useEffect(() => {
    let cancelled = false;
    (async () => {
      try {
        const data = await fetchTrips();
        const list = Array.isArray(data) ? data : data?.trips ?? [];
        if (!cancelled) {
          setTrips(list);
          if (list.length > 0) setTripId(String(list[0].id));
        }
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

  const loadExpenses = useCallback(async (id) => {
    if (!id) return;
    setExpensesLoading(true);
    try {
      const data = await fetchExpenses(id);
      setExpenses(Array.isArray(data) ? data : data?.expenses ?? []);
    } catch (err) {
      setError(err.message);
    } finally {
      setExpensesLoading(false);
    }
  }, []);

  useEffect(() => {
    setError('');
    loadExpenses(tripId);
  }, [tripId, loadExpenses]);

  const trip = trips.find((item) => String(item.id) === tripId);
  const budget = trip?.budget ?? 0;

  const spent = useMemo(
    () => expenses.reduce((sum, item) => sum + Number(item.amount || 0), 0),
    [expenses],
  );
  const remaining = budget - spent;

  // Raw (unrounded) percentage of the budget that has been spent.
  const rawPercent = budget > 0 ? (spent / budget) * 100 : 0;
  // Label: one decimal for tiny values (e.g. 0.5%), whole numbers otherwise.
  const percentLabel =
    rawPercent > 0 && rawPercent < 1 ? rawPercent.toFixed(1) : Math.round(rawPercent);
  // Bar: capped at 100%, with a 1% minimum so any spending is visible.
  const barWidth = spent > 0 ? Math.min(100, Math.max(rawPercent, 1)) : 0;
  const overBudget = budget > 0 && spent > budget;

  const byCategory = useMemo(() => {
    const totals = {};
    expenses.forEach((item) => {
      const key = item.category || 'other';
      totals[key] = (totals[key] || 0) + Number(item.amount || 0);
    });
    return Object.entries(totals).sort((a, b) => b[1] - a[1]);
  }, [expenses]);

  function handleChange(event) {
    setForm((prev) => ({ ...prev, [event.target.name]: event.target.value }));
  }

  async function handleAdd(event) {
    event.preventDefault();
    setError('');
    const amount = Number(form.amount);
    if (!amount || amount <= 0) {
      setError('Enter an amount greater than 0.');
      return;
    }
    setSaving(true);
    try {
      await addExpense(tripId, {
        category: form.category,
        amount,
        description: form.description.trim(),
      });
      setForm((prev) => ({ ...prev, amount: '', description: '' }));
      await loadExpenses(tripId);
    } catch (err) {
      setError(err.message);
    } finally {
      setSaving(false);
    }
  }

  async function handleDelete(id) {
    if (!window.confirm('Delete this expense?')) return;
    setError('');
    try {
      await deleteExpense(id);
      await loadExpenses(tripId);
    } catch (err) {
      setError(err.message);
    }
  }

  if (loading) return <div className="skeleton-grid" aria-busy="true"><div className="skeleton" /><div className="skeleton" /></div>;

  if (trips.length === 0) {
    return (
      <div className="stack">
        <section className="card empty-state">
          <div className="empty-state__icon" aria-hidden="true">💰</div>
          <h2>Plan a trip first</h2>
          <p>Expenses are tracked per trip. Create a trip, then come back to log what you spend.</p>
          <Link to="/plan-trip" className="button">Plan a trip</Link>
        </section>
      </div>
    );
  }

  return (
    <div className="stack">
      <header className="page-header">
        <div>
          <p className="page-header__eyebrow">Budget</p>
          <h1>Budget tracking</h1>
          <p className="page-header__subtitle">Log expenses and see how much of your budget is left.</p>
        </div>
        <label className="budget-select">
          <span>Trip</span>
          <select value={tripId} onChange={(event) => setTripId(event.target.value)}>
            {trips.map((item) => (
              <option key={item.id} value={item.id}>{item.tripName} · {item.destination}</option>
            ))}
          </select>
        </label>
      </header>

      <section className="budget-summary">
        <article className="card stat"><p>Total budget</p><strong>{formatMoney(budget)}</strong></article>
        <article className="card stat"><p>Spent</p><strong>{formatMoney(spent)}</strong></article>
        <article className={`card stat ${overBudget ? 'stat--danger' : ''}`}>
          <p>{overBudget ? 'Over budget by' : 'Remaining'}</p>
          <strong>{formatMoney(Math.abs(remaining))}</strong>
        </article>
      </section>

      <section className="card">
        <div className="progress__label">
          <span>{percentLabel}% of budget used</span>
          <span className="muted">{formatMoney(spent)} / {formatMoney(budget)}</span>
        </div>
        <div
          className="progress"
          role="progressbar"
          aria-valuenow={Math.round(Math.min(100, rawPercent))}
          aria-valuemin="0"
          aria-valuemax="100"
        >
          <div className={`progress__bar ${overBudget ? 'progress__bar--danger' : ''}`} style={{ width: `${barWidth}%` }} />
        </div>
      </section>

      {error && <p className="alert alert--error" role="alert">{error}</p>}

      <div className="budget-layout">
        <section className="card">
          <div className="card__header"><h2>Add expense</h2></div>
          <form className="expense-form" onSubmit={handleAdd}>
            <label>
              <span>Category</span>
              <select name="category" value={form.category} onChange={handleChange}>
                {CATEGORIES.map((item) => <option key={item} value={item}>{titleCase(item)}</option>)}
              </select>
            </label>
            <label>
              <span>Amount (₹)</span>
              <input type="number" name="amount" min="1" step="any" value={form.amount} onChange={handleChange} placeholder="e.g. 450" required />
            </label>
            <label>
              <span>Description</span>
              <input name="description" value={form.description} onChange={handleChange} placeholder="e.g. Lunch at the lake" />
            </label>
            <button className="button" disabled={saving}>{saving ? 'Adding...' : 'Add expense'}</button>
          </form>

          {byCategory.length > 0 && (
            <>
              <div className="card__header category-title"><h2>By category</h2></div>
              <ul className="category-bars">
                {byCategory.map(([category, total]) => (
                  <li key={category}>
                    <div className="category-bars__row">
                      <span>{titleCase(category)}</span>
                      <strong>{formatMoney(total)}</strong>
                    </div>
                    <div className="progress progress--thin">
                      <div className="progress__bar" style={{ width: `${spent > 0 ? (total / spent) * 100 : 0}%` }} />
                    </div>
                  </li>
                ))}
              </ul>
            </>
          )}
        </section>

        <section className="card">
          <div className="card__header">
            <h2>Expenses</h2>
            <span className="muted">{expenses.length} item{expenses.length === 1 ? '' : 's'}</span>
          </div>
          {expensesLoading ? (
            <p className="muted">Loading...</p>
          ) : expenses.length === 0 ? (
            <p className="muted">No expenses yet. Add your first one.</p>
          ) : (
            <ul className="expense-list">
              {expenses.map((item) => {
                const date = formatDate(item.expenseDate || item.date || item.createdAt);
                return (
                  <li key={item.id}>
                    <div className="expense-list__main">
                      <strong>{item.description || titleCase(item.category)}</strong>
                      <span className="muted">
                        <span className="chip">{titleCase(item.category)}</span>
                        {date && ` ${date}`}
                      </span>
                    </div>
                    <span className="expense-list__amount">{formatMoney(item.amount)}</span>
                    <button
                      type="button"
                      className="expense-list__delete"
                      onClick={() => handleDelete(item.id)}
                      aria-label="Delete expense"
                    >
                      ✕
                    </button>
                  </li>
                );
              })}
            </ul>
          )}
        </section>
      </div>
    </div>
  );
}
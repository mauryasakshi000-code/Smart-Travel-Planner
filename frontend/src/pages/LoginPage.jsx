import { useState } from 'react';
import { Link, useLocation, useNavigate } from 'react-router-dom';

import { useAuth } from '../context/AuthContext';

export default function LoginPage() {
  const { login } = useAuth();
  const navigate = useNavigate();
  const location = useLocation();
  const [form, setForm] = useState({ email: '', password: '' });
  const [error, setError] = useState('');
  const [loading, setLoading] = useState(false);

  const notice = location.state?.registered ? 'Account created. Please log in.' : '';

  function handleChange(event) {
    setForm((prev) => ({ ...prev, [event.target.name]: event.target.value }));
  }

  async function handleSubmit(event) {
    event.preventDefault();
    setError('');
    setLoading(true);
    try {
      await login({ email: form.email.trim(), password: form.password });
      navigate(location.state?.from || '/dashboard', { replace: true });
    } catch (err) {
      setError(err.message);
      setLoading(false);
    }
  }

  return (
    <div className="auth-wrap">
      <section className="auth-card card">
        <div className="auth-card__icon" aria-hidden="true">🧭</div>
        <h1>Welcome back</h1>
        <p className="auth-card__subtitle">Log in to see your trips and itineraries.</p>

        {notice && <p className="auth-alert auth-alert--success">{notice}</p>}

        <form className="auth-form" onSubmit={handleSubmit}>
          <label>
            <span>Email address</span>
            <input
              type="email"
              name="email"
              value={form.email}
              onChange={handleChange}
              placeholder="you@example.com"
              required
              autoComplete="email"
            />
          </label>

          <label>
            <span>Password</span>
            <input
              type="password"
              name="password"
              value={form.password}
              onChange={handleChange}
              placeholder="Your password"
              required
              autoComplete="current-password"
            />
          </label>

          {error && <p className="auth-alert auth-alert--error" role="alert">{error}</p>}

          <button className="button auth-submit" disabled={loading}>
            {loading ? 'Logging in...' : 'Log in'}
          </button>
        </form>

        <p className="auth-switch">
          Don't have an account? <Link to="/register">Register</Link>
        </p>
      </section>
    </div>
  );
}
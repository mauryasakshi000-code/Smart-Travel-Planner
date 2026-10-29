import { useState } from 'react';
import { Link, useNavigate } from 'react-router-dom';

import { useAuth } from '../context/AuthContext';

export default function RegisterPage() {
  const { register } = useAuth();
  const navigate = useNavigate();
  const [form, setForm] = useState({ fullName: '', email: '', password: '', confirmPassword: '' });
  const [error, setError] = useState('');
  const [loading, setLoading] = useState(false);

  function handleChange(event) {
    setForm((prev) => ({ ...prev, [event.target.name]: event.target.value }));
  }

  async function handleSubmit(event) {
    event.preventDefault();
    setError('');
    if (form.password !== form.confirmPassword) {
      setError('Password and confirmation do not match');
      return;
    }
    setLoading(true);
    try {
      await register({
        fullName: form.fullName.trim(),
        email: form.email.trim(),
        password: form.password,
        confirmPassword: form.confirmPassword,
      });
      navigate('/login', { replace: true, state: { registered: true } });
    } catch (err) {
      setError(err.message);
      setLoading(false);
    }
  }

  return (
    <div className="auth-wrap">
      <section className="auth-card card">
        <div className="auth-card__icon" aria-hidden="true">✈️</div>
        <h1>Create your account</h1>
        <p className="auth-card__subtitle">Save your trips, itineraries and expenses in one place.</p>

        <form className="auth-form" onSubmit={handleSubmit}>
          <label>
            <span>Full name</span>
            <input
              name="fullName"
              value={form.fullName}
              onChange={handleChange}
              placeholder="Your name"
              required
              minLength={2}
              autoComplete="name"
            />
          </label>

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

          <div className="auth-row">
            <label>
              <span>Password</span>
              <input
                type="password"
                name="password"
                value={form.password}
                onChange={handleChange}
                placeholder="Create a password"
                required
                minLength={8}
                autoComplete="new-password"
              />
            </label>

            <label>
              <span>Confirm password</span>
              <input
                type="password"
                name="confirmPassword"
                value={form.confirmPassword}
                onChange={handleChange}
                placeholder="Repeat password"
                required
                autoComplete="new-password"
              />
            </label>
          </div>

          <p className="auth-hint">At least 8 characters, with one letter and one number.</p>

          {error && <p className="auth-alert auth-alert--error" role="alert">{error}</p>}

          <button className="button auth-submit" disabled={loading}>
            {loading ? 'Creating account...' : 'Create account'}
          </button>
        </form>

        <p className="auth-switch">
          Already have an account? <Link to="/login">Log in</Link>
        </p>
      </section>
    </div>
  );
}
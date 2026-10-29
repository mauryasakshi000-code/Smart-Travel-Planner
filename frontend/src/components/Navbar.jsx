import { Link, NavLink, useNavigate } from 'react-router-dom';

import { useAuth } from '../context/AuthContext';

export default function Navbar() {
  const { user, logout } = useAuth();
  const navigate = useNavigate();

  async function handleLogout() {
    await logout();
    navigate('/login', { replace: true });
  }

  return (
    <header className="navbar">
      <Link to="/" className="navbar__brand">
        <span aria-hidden="true">🧭</span> Smart Travel Planner
      </Link>

      <nav className="navbar__links">
        <NavLink to="/" end>Home</NavLink>
        {user ? (
          <>
            <NavLink to="/dashboard">Dashboard</NavLink>
            <NavLink to="/plan-trip">Plan Trip</NavLink>
            <NavLink to="/trips">My Trips</NavLink>
            <NavLink to="/budget">Budget</NavLink>
            <NavLink to="/stories">Stories</NavLink>
            <span className="navbar__user" title={user.fullName}>{user.fullName}</span>
            <button type="button" className="button" onClick={handleLogout}>
              Log out
            </button>
          </>
        ) : (
          <>
            <NavLink to="/login">Login</NavLink>
            <NavLink to="/register" className="button">Register</NavLink>
          </>
        )}
      </nav>
    </header>
  );
}
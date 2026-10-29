import { Navigate, Outlet, useLocation } from 'react-router-dom';

import { useAuth } from '../context/AuthContext';

// Only for logged-in users. Others are sent to /login and returned here afterwards.
export function ProtectedRoute() {
  const { user, loading } = useAuth();
  const location = useLocation();

  if (loading) return <div className="skeleton-grid" aria-busy="true"><div className="skeleton" /></div>;
  if (!user) return <Navigate to="/login" replace state={{ from: location.pathname }} />;
  return <Outlet />;
}

// Only for logged-out users (login/register). Logged-in users go to the dashboard.
export function PublicOnlyRoute() {
  const { user, loading } = useAuth();

  if (loading) return <div className="skeleton-grid" aria-busy="true"><div className="skeleton" /></div>;
  if (user) return <Navigate to="/dashboard" replace />;
  return <Outlet />;
}
import { Navigate, Route, Routes } from 'react-router-dom';

import Navbar from './components/Navbar';
import { ProtectedRoute, PublicOnlyRoute } from './components/RouteGuards';
import BudgetPage from './pages/BudgetPage';
import DashboardPage from './pages/DashboardPage';
import HomePage from './pages/HomePage';
import LoginPage from './pages/LoginPage';
import NotFoundPage from './pages/NotFoundPage';
import RegisterPage from './pages/RegisterPage';
import StoriesPage from './pages/StoriesPage';
import TripDetailPage from './pages/TripDetailPage';
import TripPlannerPage from './pages/TripPlannerPage';
import TripsPage from './pages/TripsPage';

export default function App() {
  return (
    <div className="app-shell">
      <Navbar />
      <main className="app-main">
        <Routes>
          <Route path="/" element={<HomePage />} />

          <Route element={<PublicOnlyRoute />}>
            <Route path="/login" element={<LoginPage />} />
            <Route path="/register" element={<RegisterPage />} />
          </Route>

          <Route element={<ProtectedRoute />}>
            <Route path="/dashboard" element={<DashboardPage />} />
            <Route path="/plan-trip" element={<TripPlannerPage />} />
            <Route path="/trips" element={<TripsPage />} />
            <Route path="/trips/new" element={<Navigate to="/plan-trip" replace />} />
            <Route path="/trips/:id" element={<TripDetailPage />} />
            <Route path="/budget" element={<BudgetPage />} />
            <Route path="/stories" element={<StoriesPage />} />
          </Route>

          <Route path="*" element={<NotFoundPage />} />
        </Routes>
      </main>
      <footer className="app-footer">
        Smart Travel Planner · React + C++ + PostgreSQL
      </footer>
    </div>
  );
}
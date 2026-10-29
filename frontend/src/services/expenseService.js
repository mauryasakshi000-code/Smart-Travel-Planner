import { apiRequest } from './apiClient';

export function fetchExpenses(tripId) {
  return apiRequest(`/trips/${tripId}/expenses`);
}

export function addExpense(tripId, body) {
  return apiRequest(`/trips/${tripId}/expenses`, { method: 'POST', body });
}

export function updateExpense(id, body) {
  return apiRequest(`/expenses/${id}`, { method: 'PUT', body });
}

export function deleteExpense(id) {
  return apiRequest(`/expenses/${id}`, { method: 'DELETE' });
}

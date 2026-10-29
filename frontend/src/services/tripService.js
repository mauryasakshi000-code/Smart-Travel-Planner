import { apiRequest } from './apiClient';

export function fetchActivities() {
  return apiRequest('/activities');
}

export function fetchTrips() {
  return apiRequest('/trips');
}

export function fetchTrip(id) {
  return apiRequest(`/trips/${id}`);
}

export function createTrip(body) {
  return apiRequest('/trips', { method: 'POST', body, timeoutMs: 30000 });
}

export function updateTrip(id, body) {
  return apiRequest(`/trips/${id}`, { method: 'PUT', body });
}

export function deleteTrip(id) {
  return apiRequest(`/trips/${id}`, { method: 'DELETE' });
}

export function generateItinerary(id) {
  return apiRequest(`/trips/${id}/generate`, { method: 'POST', timeoutMs: 30000 });
}

export function fetchItinerary(id) {
  return apiRequest(`/trips/${id}/itinerary`);
}

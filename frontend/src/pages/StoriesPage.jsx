import { useCallback, useEffect, useState } from 'react';

import {
  addPlace,
  createStory,
  deletePlace,
  deleteStory,
  fetchMyStories,
  fetchPlaces,
  fetchPublicStories,
  publishStory,
  updateStory,
} from '../services/storyService';
import { fetchTrips } from '../services/tripService';

const TABS = [
  { id: 'mine', label: 'My stories' },
  { id: 'community', label: 'Community' },
  { id: 'places', label: 'Saved places' },
];

const EMPTY_EDITOR = { id: null, title: '', content: '', coverImageUrl: '', tripId: '' };
const EMPTY_PLACE = { placeName: '', destination: '', notes: '' };

function formatDate(value) {
  if (!value) return '';
  const date = new Date(String(value).replace(' ', 'T'));
  if (Number.isNaN(date.getTime())) return '';
  return date.toLocaleDateString('en-IN', { day: 'numeric', month: 'short', year: 'numeric' });
}

function StoryCard({ story, owner, onEdit, onPublish, onDelete }) {
  const [open, setOpen] = useState(false);
  const long = story.content.length > 240;
  const text = open || !long ? story.content : `${story.content.slice(0, 240).trimEnd()}…`;

  return (
    <article className="story-card">
      {story.coverImageUrl && (
        <img
          className="story-card__cover"
          src={story.coverImageUrl}
          alt=""
          loading="lazy"
          onError={(event) => {
            event.currentTarget.style.display = 'none';
          }}
        />
      )}
      <div className="story-card__body">
        <div className="story-card__top">
          <h2>{story.title}</h2>
          {owner && (
            <span className={`trip-badge ${story.published ? 'trip-badge--completed' : 'trip-badge--draft'}`}>
              {story.published ? 'Published' : 'Draft'}
            </span>
          )}
        </div>
        <p className="story-card__meta">
          {!owner && story.author && `By ${story.author} · `}
          {story.destination && `📍 ${story.destination} · `}
          {formatDate(story.createdAt)}
        </p>
        <p className="story-card__text">{text}</p>
        {long && (
          <button type="button" className="link-button" onClick={() => setOpen((value) => !value)}>
            {open ? 'Show less' : 'Read more'}
          </button>
        )}
        {owner && (
          <div className="story-card__actions">
            <button type="button" className="button button--small" onClick={() => onEdit(story)}>Edit</button>
            {!story.published && (
              <button type="button" className="button button--small" onClick={() => onPublish(story)}>Publish</button>
            )}
            <button type="button" className="button button--small button--danger" onClick={() => onDelete(story)}>
              Delete
            </button>
          </div>
        )}
      </div>
    </article>
  );
}

export default function StoriesPage() {
  const [tab, setTab] = useState('mine');
  const [mine, setMine] = useState([]);
  const [community, setCommunity] = useState([]);
  const [places, setPlaces] = useState([]);
  const [trips, setTrips] = useState([]);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState('');
  const [saving, setSaving] = useState(false);
  const [editor, setEditor] = useState(null);
  const [placeForm, setPlaceForm] = useState(EMPTY_PLACE);

  const load = useCallback(async (which) => {
    setLoading(true);
    setError('');
    try {
      if (which === 'mine') {
        const data = await fetchMyStories();
        setMine(data.stories ?? []);
      } else if (which === 'community') {
        const data = await fetchPublicStories();
        setCommunity(data.stories ?? []);
      } else {
        const data = await fetchPlaces();
        setPlaces(data.places ?? []);
      }
    } catch (err) {
      setError(err.message);
    } finally {
      setLoading(false);
    }
  }, []);

  useEffect(() => {
    load(tab);
  }, [tab, load]);

  useEffect(() => {
    fetchTrips()
      .then((data) => setTrips(Array.isArray(data) ? data : data?.trips ?? []))
      .catch(() => {});
  }, []);

  function openEdit(story) {
    setEditor({
      id: story.id,
      title: story.title,
      content: story.content,
      coverImageUrl: story.coverImageUrl || '',
      tripId: '',
    });
    window.scrollTo({ top: 0, behavior: 'smooth' });
  }

  function handleEditorChange(event) {
    setEditor((prev) => ({ ...prev, [event.target.name]: event.target.value }));
  }

  async function handleSave(event) {
    event.preventDefault();
    setError('');
    setSaving(true);
    try {
      const body = {
        title: editor.title.trim(),
        content: editor.content.trim(),
        coverImageUrl: editor.coverImageUrl.trim(),
      };
      if (editor.id) {
        await updateStory(editor.id, body);
      } else {
        if (editor.tripId) body.tripId = Number(editor.tripId);
        await createStory(body);
      }
      setEditor(null);
      await load('mine');
    } catch (err) {
      setError(err.message);
    } finally {
      setSaving(false);
    }
  }

  async function handlePublish(story) {
    if (!window.confirm('Publish this story? Everyone using the app will be able to read it.')) return;
    setError('');
    try {
      await publishStory(story.id);
      await load('mine');
    } catch (err) {
      setError(err.message);
    }
  }

  async function handleDelete(story) {
    if (!window.confirm('Delete this story permanently?')) return;
    setError('');
    try {
      await deleteStory(story.id);
      await load('mine');
    } catch (err) {
      setError(err.message);
    }
  }

  function handlePlaceChange(event) {
    setPlaceForm((prev) => ({ ...prev, [event.target.name]: event.target.value }));
  }

  async function handleAddPlace(event) {
    event.preventDefault();
    setError('');
    setSaving(true);
    try {
      await addPlace({
        placeName: placeForm.placeName.trim(),
        destination: placeForm.destination.trim(),
        notes: placeForm.notes.trim(),
      });
      setPlaceForm(EMPTY_PLACE);
      await load('places');
    } catch (err) {
      setError(err.message);
    } finally {
      setSaving(false);
    }
  }

  async function handleDeletePlace(place) {
    if (!window.confirm(`Remove "${place.placeName}" from saved places?`)) return;
    setError('');
    try {
      await deletePlace(place.id);
      await load('places');
    } catch (err) {
      setError(err.message);
    }
  }

  return (
    <div className="stack">
      <header className="page-header">
        <div>
          <p className="page-header__eyebrow">Travel stories</p>
          <h1>Share your journeys</h1>
          <p className="page-header__subtitle">Write about your trips, read what others shared, and keep a list of places to visit.</p>
        </div>
        {tab === 'mine' && !editor && (
          <button type="button" className="button" onClick={() => setEditor({ ...EMPTY_EDITOR })}>
            + Write a story
          </button>
        )}
      </header>

      <div className="tabs" role="tablist">
        {TABS.map((item) => (
          <button
            key={item.id}
            type="button"
            role="tab"
            aria-selected={tab === item.id}
            className={`tabs__tab ${tab === item.id ? 'tabs__tab--active' : ''}`}
            onClick={() => {
              setTab(item.id);
              setEditor(null);
            }}
          >
            {item.label}
          </button>
        ))}
      </div>

      {error && <p className="alert alert--error" role="alert">{error}</p>}

      {tab === 'mine' && editor && (
        <section className="card">
          <div className="card__header">
            <h2>{editor.id ? 'Edit story' : 'New story'}</h2>
            <button type="button" className="link-button" onClick={() => setEditor(null)}>Cancel</button>
          </div>
          <form className="story-form" onSubmit={handleSave}>
            <label>
              <span>Title</span>
              <input name="title" value={editor.title} onChange={handleEditorChange} placeholder="e.g. A weekend in Ooty" required />
            </label>
            {!editor.id && (
              <label>
                <span>Related trip (optional)</span>
                <select name="tripId" value={editor.tripId} onChange={handleEditorChange}>
                  <option value="">No trip</option>
                  {trips.map((trip) => (
                    <option key={trip.id} value={trip.id}>{trip.tripName} · {trip.destination}</option>
                  ))}
                </select>
              </label>
            )}
            <label>
              <span>Cover image link (optional)</span>
              <input
                type="url"
                name="coverImageUrl"
                value={editor.coverImageUrl}
                onChange={handleEditorChange}
                placeholder="https://..."
              />
            </label>
            <label>
              <span>Your story</span>
              <textarea
                name="content"
                rows={9}
                value={editor.content}
                onChange={handleEditorChange}
                placeholder="What did you see, eat and feel?"
                required
              />
            </label>
            <p className="story-form__hint">New stories are saved as private drafts. Publish them from the list when you're ready.</p>
            <button className="button" disabled={saving}>
              {saving ? 'Saving...' : editor.id ? 'Save changes' : 'Save as draft'}
            </button>
          </form>
        </section>
      )}

      {loading ? (
        <div className="skeleton-grid" aria-busy="true"><div className="skeleton" /><div className="skeleton" /></div>
      ) : tab === 'mine' ? (
        mine.length === 0 && !editor ? (
          <section className="card empty-state">
            <div className="empty-state__icon" aria-hidden="true">✍️</div>
            <h2>No stories yet</h2>
            <p>Write about a trip. It stays private until you publish it.</p>
            <button type="button" className="button" onClick={() => setEditor({ ...EMPTY_EDITOR })}>
              Write your first story
            </button>
          </section>
        ) : (
          <div className="story-grid">
            {mine.map((story) => (
              <StoryCard
                key={story.id}
                story={story}
                owner
                onEdit={openEdit}
                onPublish={handlePublish}
                onDelete={handleDelete}
              />
            ))}
          </div>
        )
      ) : tab === 'community' ? (
        community.length === 0 ? (
          <section className="card empty-state">
            <div className="empty-state__icon" aria-hidden="true">🌍</div>
            <h2>Nothing published yet</h2>
            <p>Stories that people publish will show up here.</p>
          </section>
        ) : (
          <div className="story-grid">
            {community.map((story) => (
              <StoryCard key={story.id} story={story} owner={false} />
            ))}
          </div>
        )
      ) : (
        <div className="budget-layout">
          <section className="card">
            <div className="card__header"><h2>Save a place</h2></div>
            <form className="story-form" onSubmit={handleAddPlace}>
              <label>
                <span>Place name</span>
                <input name="placeName" value={placeForm.placeName} onChange={handlePlaceChange} placeholder="e.g. Doddabetta Peak" required />
              </label>
              <label>
                <span>Destination</span>
                <input name="destination" value={placeForm.destination} onChange={handlePlaceChange} placeholder="e.g. Ooty" />
              </label>
              <label>
                <span>Notes</span>
                <textarea name="notes" rows={3} value={placeForm.notes} onChange={handlePlaceChange} placeholder="Best time to go, things to try..." />
              </label>
              <button className="button" disabled={saving}>{saving ? 'Saving...' : 'Save place'}</button>
            </form>
          </section>

          <section className="card">
            <div className="card__header">
              <h2>Saved places</h2>
              <span className="muted">{places.length} saved</span>
            </div>
            {places.length === 0 ? (
              <p className="muted">No saved places yet.</p>
            ) : (
              <ul className="place-list">
                {places.map((place) => (
                  <li key={place.id}>
                    <div>
                      <strong>📍 {place.placeName}</strong>
                      {place.destination && <span className="chip">{place.destination}</span>}
                      {place.notes && <p>{place.notes}</p>}
                    </div>
                    <button
                      type="button"
                      className="expense-list__delete"
                      onClick={() => handleDeletePlace(place)}
                      aria-label="Remove place"
                    >
                      ✕
                    </button>
                  </li>
                ))}
              </ul>
            )}
          </section>
        </div>
      )}
    </div>
  );
}
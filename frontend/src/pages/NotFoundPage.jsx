import { Link } from 'react-router-dom';

export default function NotFoundPage() {
  return (
    <section className="card not-found">
      <h1>Page not found</h1>
      <p>The page you are looking for doesn&apos;t exist.</p>
      <Link to="/" className="button">
        Back to home
      </Link>
    </section>
  );
}

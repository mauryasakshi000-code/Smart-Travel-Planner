const LABELS = {
  ok: 'Online',
  error: 'Offline',
  pending: 'Checking…',
};

export default function StatusRow({ name, detail, state }) {
  return (
    <li className="status-row">
      <div>
        <p className="status-row__name">{name}</p>
        <p className="status-row__detail">{detail}</p>
      </div>
      <span className={`status-pill status-pill--${state}`}>{LABELS[state]}</span>
    </li>
  );
}

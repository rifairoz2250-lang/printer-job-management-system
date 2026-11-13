/**
 * ============================================================================
 * File: app.js
 * Project: Printer Job Management System using Queue in C
 * Description: Client-side JavaScript connecting the modern dashboard to the
 *              C Backend HTTP REST API. Manages real-time UI updates, FIFO
 *              queue animations, form submissions, and alert notifications.
 * ============================================================================
 */

const API_BASE = (window.location.origin && window.location.origin !== "null" && !window.location.origin.startsWith("file:"))
  ? window.location.origin 
  : "http://localhost:8080";

// State cache
let lastKnownState = null;
let alertTimeout = null;

/**
 * Initialize on page load
 */
document.addEventListener("DOMContentLoaded", () => {
  fetchQueueState();
  // Poll queue state every 3 seconds for live synchronization
  setInterval(fetchQueueState, 3000);
});

/**
 * Display alert notification banner (Success, Danger, Info)
 */
function showAlert(title, message, type = "info") {
  const banner = document.getElementById("alert-banner");
  const titleEl = document.getElementById("alert-title");
  const msgEl = document.getElementById("alert-message");
  const iconEl = document.getElementById("alert-icon");

  if (!banner || !titleEl || !msgEl || !iconEl) return;

  banner.className = `alert-banner alert-${type}`;
  titleEl.textContent = title;
  msgEl.textContent = message;

  if (type === "success") {
    iconEl.textContent = "✅";
  } else if (type === "danger") {
    iconEl.textContent = "⚠️";
  } else {
    iconEl.textContent = "ℹ️";
  }

  banner.classList.remove("hidden");

  if (alertTimeout) clearTimeout(alertTimeout);
  alertTimeout = setTimeout(() => {
    closeAlert();
  }, 6000);
}

function closeAlert() {
  const banner = document.getElementById("alert-banner");
  if (banner) {
    banner.classList.add("hidden");
  }
}

/**
 * Fetch current Queue State from C HTTP Server (/api/jobs)
 */
async function fetchQueueState() {
  try {
    const res = await fetch(`${API_BASE}/api/jobs`);
    if (!res.ok) throw new Error(`HTTP error: ${res.status}`);

    const data = await res.json();
    lastKnownState = data;
    renderDashboard(data);
    updateConnectionBadge(true);
  } catch (err) {
    console.warn("Could not reach C Web Server:", err.message);
    updateConnectionBadge(false);
  }
}

/**
 * Update connection status pill
 */
function updateConnectionBadge(isConnected) {
  const badge = document.getElementById("connection-badge");
  if (!badge) return;

  if (isConnected) {
    badge.className = "badge badge-status";
    badge.innerHTML = `<span class="pulse-dot"></span> C Backend: Active`;
  } else {
    badge.className = "badge";
    badge.style.backgroundColor = "#fee2e2";
    badge.style.color = "#991b1b";
    badge.style.border = "1px solid #fecaca";
    badge.innerHTML = `⚠️ C Backend: Offline`;
  }
}

/**
 * Render complete dashboard with fresh data from C Queue
 */
function renderDashboard(data) {
  const count = data.count || 0;
  const capacity = data.capacity || 10;
  const jobs = data.jobs || [];

  // 1. Update Metrics Cards
  document.getElementById("count-pending").textContent = count;
  document.getElementById("capacity-val").textContent = capacity;

  const pct = Math.round((count / capacity) * 100);
  const bar = document.getElementById("capacity-bar");
  if (bar) bar.style.width = `${pct}%`;

  const capText = document.getElementById("capacity-text");
  if (capText) {
    if (count === 0) {
      capText.textContent = "Queue buffer is completely empty (0% used)";
    } else if (count >= capacity) {
      capText.textContent = `Queue OVERFLOW risk! 100% capacity reached`;
      capText.style.color = "var(--danger)";
    } else {
      capText.textContent = `${pct}% used (${capacity - count} slots available)`;
      capText.style.color = "var(--text-muted)";
    }
  }

  // Front Job Metric
  const frontDisplay = document.getElementById("front-job-display");
  const frontIdxVal = document.getElementById("front-index-val");
  if (jobs.length > 0) {
    const f = jobs[0];
    frontDisplay.innerHTML = `<span>#${f.job_id} &mdash; <strong>${escapeHtml(f.doc_name)}</strong> (${f.pages}p)</span>`;
    frontIdxVal.textContent = "0";
  } else {
    frontDisplay.innerHTML = `<span class="text-muted">No jobs in queue</span>`;
    frontIdxVal.textContent = "-1";
  }

  // Rear Job Metric
  const rearDisplay = document.getElementById("rear-job-display");
  const rearIdxVal = document.getElementById("rear-index-val");
  if (jobs.length > 0) {
    const r = jobs[jobs.length - 1];
    rearDisplay.innerHTML = `<span>#${r.job_id} &mdash; <strong>${escapeHtml(r.doc_name)}</strong> (${r.pages}p)</span>`;
    rearIdxVal.textContent = String(jobs.length - 1);
  } else {
    rearDisplay.innerHTML = `<span class="text-muted">No jobs in queue</span>`;
    rearIdxVal.textContent = "-1";
  }

  // Lifetime Stats
  document.getElementById("stat-printed").textContent = data.total_printed || 0;
  document.getElementById("stat-cancelled").textContent = data.total_cancelled || 0;

  const lastPrintedFooter = document.getElementById("last-printed-footer");
  if (data.last_printed) {
    lastPrintedFooter.innerHTML = `Last: <strong>#${data.last_printed.job_id}</strong> - ${escapeHtml(data.last_printed.doc_name)}`;
  } else {
    lastPrintedFooter.textContent = "No document printed yet";
  }

  // 2. Render Visual Array Slots (Contiguous FIFO Array)
  renderQueueTrack(jobs, capacity);

  // 3. Render Pending Jobs Table
  renderTable(jobs);
}

/**
 * Render visual FIFO Queue Slots track
 */
function renderQueueTrack(jobs, capacity) {
  const track = document.getElementById("queue-slots-track");
  if (!track) return;

  track.innerHTML = "";

  for (let i = 0; i < capacity; i++) {
    const job = jobs[i];
    const slot = document.createElement("div");

    if (job) {
      const isFront = (i === 0);
      const isRear = (i === jobs.length - 1);

      slot.className = `slot-box occupied ${isFront ? 'is-front' : ''} ${isRear ? 'is-rear' : ''}`;

      let tagHtml = "";
      if (isFront && isRear) {
        tagHtml = `<span class="slot-badge-tag tag-front">FRONT & REAR</span>`;
      } else if (isFront) {
        tagHtml = `<span class="slot-badge-tag tag-front">FRONT (Next)</span>`;
      } else if (isRear) {
        tagHtml = `<span class="slot-badge-tag tag-rear">REAR (Newest)</span>`;
      }

      slot.innerHTML = `
        <div class="slot-index">[Index ${i}]</div>
        ${tagHtml}
        <div class="slot-content">
          <div class="slot-job-id">#${job.job_id}</div>
          <div class="slot-doc-name" title="${escapeHtml(job.doc_name)}">${escapeHtml(job.doc_name)}</div>
        </div>
        <div class="slot-index">${job.pages} pgs</div>
      `;
    } else {
      slot.className = "slot-box";
      slot.innerHTML = `
        <div class="slot-index">[Index ${i}]</div>
        <div class="slot-empty-label">Empty</div>
        <div class="slot-index">&mdash;</div>
      `;
    }

    track.appendChild(slot);
  }
}

/**
 * Render Pending Jobs Table
 */
function renderTable(jobs) {
  const tbody = document.getElementById("jobs-tbody");
  const emptyState = document.getElementById("empty-state");
  const table = document.getElementById("jobs-table");

  if (!tbody || !emptyState || !table) return;

  tbody.innerHTML = "";

  if (jobs.length === 0) {
    emptyState.classList.remove("hidden");
    table.style.display = "none";
    return;
  }

  emptyState.classList.add("hidden");
  table.style.display = "table";

  jobs.forEach((job, index) => {
    const tr = document.createElement("tr");
    const isFront = (index === 0);
    const isRear = (index === jobs.length - 1);

    if (isFront) tr.className = "row-front";

    let posLabel = `#${index + 1}`;
    let posBadge = "";
    if (isFront && isRear) {
      posBadge = `<span class="pos-front-tag">FRONT/REAR</span>`;
    } else if (isFront) {
      posBadge = `<span class="pos-front-tag">FRONT (1st)</span>`;
    } else if (isRear) {
      posBadge = `<span class="pos-rear-tag">REAR</span>`;
    }

    tr.innerHTML = `
      <td>
        <div class="pos-tag">
          <span>${posLabel}</span>
          ${posBadge}
        </div>
      </td>
      <td>
        <span class="table-job-badge">#${job.job_id}</span>
      </td>
      <td>
        <strong>${escapeHtml(job.doc_name)}</strong>
      </td>
      <td>${job.pages} pages</td>
      <td><code>${job.submitted_at}</code></td>
      <td class="text-right">
        <button class="btn btn-outline btn-sm text-danger" onclick="quickCancel(${job.job_id})" title="Cancel this job">
          🗑️ Cancel
        </button>
      </td>
    `;

    tbody.appendChild(tr);
  });
}

/**
 * Handle Add Print Job Form Submission
 */
async function handleAddJob(event) {
  event.preventDefault();

  const docInput = document.getElementById("doc-name-input");
  const pagesInput = document.getElementById("pages-input");
  const btn = document.getElementById("btn-add-job");

  const doc_name = docInput.value.trim();
  const pages = parseInt(pagesInput.value, 10);

  if (!doc_name) {
    showAlert("Validation Error", "Document name cannot be empty.", "danger");
    docInput.focus();
    return;
  }

  if (isNaN(pages) || pages < 1) {
    showAlert("Validation Error", "Number of pages must be at least 1.", "danger");
    pagesInput.focus();
    return;
  }

  btn.disabled = true;
  btn.innerHTML = `<span class="btn-icon">⏳</span> Adding to Queue...`;

  try {
    const res = await fetch(`${API_BASE}/api/jobs`, {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ doc_name, pages })
    });

    const data = await res.json();

    if (res.ok && data.success) {
      showAlert("Job Added (Enqueue)", `${data.message} Document: "${data.job.doc_name}"`, "success");
      docInput.value = "";
      pagesInput.value = "5";
      fetchQueueState();
    } else {
      showAlert("Queue Rejection", data.message || "Could not add job.", "danger");
    }
  } catch (err) {
    showAlert("Connection Error", "Failed to communicate with C backend engine.", "danger");
  } finally {
    btn.disabled = false;
    btn.innerHTML = `<span class="btn-icon">➕</span> Add Job to Queue`;
  }
}

/**
 * Handle Print Next Job (Dequeue - FIFO)
 */
async function handlePrintNext() {
  const btn = document.getElementById("btn-print-next");
  btn.disabled = true;
  btn.innerHTML = `<span class="btn-icon">⏳</span> Printing...`;

  try {
    const res = await fetch(`${API_BASE}/api/print`, {
      method: "POST",
      headers: { "Content-Type": "application/json" }
    });

    const data = await res.json();

    if (res.ok && data.success) {
      showAlert("Document Printed (FIFO Dequeue)", 
        `Now Printing Job #${data.printed_job.job_id} ("${data.printed_job.doc_name}", ${data.printed_job.pages} pages). Processed from FRONT in FIFO order!`, 
        "success");
      fetchQueueState();
    } else {
      showAlert("Cannot Print (Underflow)", data.message || "Queue is empty.", "danger");
    }
  } catch (err) {
    showAlert("Connection Error", "Failed to connect to C backend engine.", "danger");
  } finally {
    btn.disabled = false;
    btn.innerHTML = `<span class="btn-icon">🖨️</span> Print Next Job`;
  }
}

/**
 * Handle Cancel Job Form Submission
 */
async function handleCancelJob(event) {
  event.preventDefault();
  const input = document.getElementById("cancel-job-id");
  const job_id = parseInt(input.value, 10);

  if (isNaN(job_id) || job_id <= 0) {
    showAlert("Validation Error", "Please enter a valid positive Job ID.", "danger");
    input.focus();
    return;
  }

  await performCancel(job_id);
  input.value = "";
}

/**
 * Quick Cancel directly from Table Row button
 */
async function quickCancel(job_id) {
  if (confirm(`Are you sure you want to cancel Job #${job_id}?`)) {
    await performCancel(job_id);
  }
}

/**
 * Send Cancel request to C backend
 */
async function performCancel(job_id) {
  try {
    const res = await fetch(`${API_BASE}/api/cancel`, {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ job_id })
    });

    const data = await res.json();

    if (res.ok && data.success) {
      showAlert("Job Cancelled", data.message, "info");
      fetchQueueState();
    } else {
      showAlert("Cancel Failed", data.message || "Job ID not found in queue.", "danger");
    }
  } catch (err) {
    showAlert("Connection Error", "Failed to connect to C backend engine.", "danger");
  }
}

/**
 * Safe HTML escaping to prevent XSS
 */
function escapeHtml(str) {
  if (!str) return "";
  return String(str)
    .replace(/&/g, "&amp;")
    .replace(/</g, "&lt;")
    .replace(/>/g, "&gt;")
    .replace(/"/g, "&quot;")
    .replace(/'/g, "&#039;");
}

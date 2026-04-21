/**
 * bench_drt.cc �?Microbenchmark for dynamic_routing_table
 *
 * Measures:
 *   1. Route registration (insert) throughput
 *   2. Route lookup (find) throughput �?both hits and misses
 *   3. Peak RSS (memory footprint)
 *
 * Build (Linux/WSL):
 *   g++ -O2 -std=c++20 -o bench_drt drt.cc
 *
 * Usage:
 *   ./bench_drt [num_iterations]    (default: 5)
 */

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <numeric>
#include <random>
#include <string>
#include <vector>

// For RSS measurement on Linux
#ifdef __linux__
#include <fstream>
#include <unistd.h>
#endif

#include "../http_server/dynamic_routing_table.hh"

// ---- Route value type matching lithium's usage ----

using handler_fn = void (*)(int);

struct route_value {
  int method;
  handler_fn handler;
};

// ---- Helpers ----

static void dummy_handler(int) {}

static long get_rss_kb() {
#ifdef __linux__
  std::ifstream status("/proc/self/status");
  std::string line;
  while (std::getline(status, line)) {
    if (line.rfind("VmRSS:", 0) == 0) {
      long kb = 0;
      std::sscanf(line.c_str(), "VmRSS: %ld", &kb);
      return kb;
    }
  }
#endif
  return -1;
}

using hi_res_clock = std::chrono::high_resolution_clock;
using ns = std::chrono::nanoseconds;

// ---- Route generators ----

static std::vector<std::string> generate_routes() {
  // Mix of static, parameterized, and deep paths �?realistic REST API surface
  std::vector<std::string> routes;

  // Static CRUD-style routes (typical REST API)
  const char* resources[] = {"users", "posts", "comments", "articles", "products",
                             "orders", "invoices", "sessions", "teams", "projects",
                             "tasks", "events", "files", "images", "tags",
                             "categories", "settings", "notifications", "messages", "logs"};

  for (auto res : resources) {
    routes.push_back(std::string("/api/v1/") + res);
    routes.push_back(std::string("/api/v1/") + res + "/{{id}}");
    routes.push_back(std::string("/api/v1/") + res + "/{{id}}/details");
    routes.push_back(std::string("/api/v1/") + res + "/{{id}}/edit");
    routes.push_back(std::string("/api/v1/") + res + "/{{id}}/delete");
    routes.push_back(std::string("/api/v2/") + res);
    routes.push_back(std::string("/api/v2/") + res + "/{{id}}");
  }

  // Deeper nested routes
  routes.push_back("/api/v1/users/{{user_id}}/posts/{{post_id}}/comments");
  routes.push_back("/api/v1/users/{{user_id}}/posts/{{post_id}}/comments/{{comment_id}}");
  routes.push_back("/api/v1/teams/{{team_id}}/projects/{{project_id}}/tasks");
  routes.push_back("/api/v1/teams/{{team_id}}/projects/{{project_id}}/tasks/{{task_id}}");
  routes.push_back("/api/v1/organizations/{{org_id}}/teams/{{team_id}}/members");

  // Static utility routes
  routes.push_back("/health");
  routes.push_back("/metrics");
  routes.push_back("/api/v1/auth/login");
  routes.push_back("/api/v1/auth/logout");
  routes.push_back("/api/v1/auth/refresh");
  routes.push_back("/api/v1/search");
  routes.push_back("/api/v1/export");

  return routes;
}

static std::vector<std::string> generate_lookup_urls(std::mt19937& rng) {
  // Concrete URLs that would match the parameterized routes
  std::vector<std::string> urls;

  const char* resources[] = {"users", "posts", "comments", "articles", "products",
                             "orders", "invoices", "sessions", "teams", "projects"};

  for (auto res : resources) {
    for (int id = 1; id <= 50; id++) {
      urls.push_back(std::string("/api/v1/") + res + "/" + std::to_string(id));
      urls.push_back(std::string("/api/v1/") + res + "/" + std::to_string(id) + "/details");
    }
    urls.push_back(std::string("/api/v1/") + res);
    urls.push_back(std::string("/api/v2/") + res);
  }

  // Some deep nested lookups
  for (int i = 1; i <= 20; i++) {
    urls.push_back("/api/v1/users/" + std::to_string(i) + "/posts/" +
                   std::to_string(i * 10) + "/comments");
  }

  // Static routes
  urls.push_back("/health");
  urls.push_back("/metrics");
  urls.push_back("/api/v1/auth/login");
  urls.push_back("/api/v1/search");

  // Shuffle for realistic access pattern
  std::shuffle(urls.begin(), urls.end(), rng);
  return urls;
}

static std::vector<std::string> generate_miss_urls() {
  return {
    "/api/v3/users",
    "/api/v1/nonexistent",
    "/api/v1/users/123/unknown",
    "/totally/wrong/path",
    "/api/v1/users/123/posts/456/comments/789/replies",
    "/",
    "/api",
  };
}

// ---- Benchmark runners ----

struct bench_result {
  double insert_ns_per_op;
  double lookup_hit_ns_per_op;
  double lookup_miss_ns_per_op;
  long rss_after_insert_kb;
};

static bench_result run_once(const std::vector<std::string>& routes,
                            const std::vector<std::string>& lookup_urls,
                            const std::vector<std::string>& miss_urls,
                            int lookup_multiplier) {
  bench_result result{};

  long rss_before = get_rss_kb();

  // ---- Insert benchmark ----
  li::dynamic_routing_table<route_value> table;
  auto t0 = hi_res_clock::now();
  for (auto& r : routes) {
    auto& v = table[r];
    v.method = 1;
    v.handler = dummy_handler;
  }
  auto t1 = hi_res_clock::now();
  result.insert_ns_per_op =
      (double)std::chrono::duration_cast<ns>(t1 - t0).count() / routes.size();

  result.rss_after_insert_kb = get_rss_kb() - rss_before;

  // ---- Lookup hit benchmark ----
  volatile int sink = 0; // prevent optimizer from eliminating lookups
  auto t2 = hi_res_clock::now();
  for (int rep = 0; rep < lookup_multiplier; rep++) {
    for (auto& url : lookup_urls) {
      auto it = table.find(url);
      if (it != table.end())
        sink += it->second.method;
    }
  }
  auto t3 = hi_res_clock::now();
  long total_hits = (long)lookup_urls.size() * lookup_multiplier;
  result.lookup_hit_ns_per_op =
      (double)std::chrono::duration_cast<ns>(t3 - t2).count() / total_hits;

  // ---- Lookup miss benchmark ----
  auto t4 = hi_res_clock::now();
  for (int rep = 0; rep < lookup_multiplier * 10; rep++) {
    for (auto& url : miss_urls) {
      auto it = table.find(url);
      if (it != table.end())
        sink += it->second.method;
    }
  }
  auto t5 = hi_res_clock::now();
  long total_misses = (long)miss_urls.size() * lookup_multiplier * 10;
  result.lookup_miss_ns_per_op =
      (double)std::chrono::duration_cast<ns>(t5 - t4).count() / total_misses;

  return result;
}

// ---- Wide-node route generators (20+ children under one parent) ----

static std::vector<std::string> generate_wide_routes() {
  std::vector<std::string> routes;

  // 24 static endpoints under /api/v1/ (wide fan-out at one level)
  const char* endpoints[] = {
    "users", "posts", "comments", "articles", "products", "orders",
    "invoices", "sessions", "teams", "projects", "tasks", "events",
    "files", "images", "tags", "categories", "settings", "notifications",
    "messages", "logs", "analytics", "reports", "dashboards", "webhooks"
  };

  for (auto ep : endpoints) {
    routes.push_back(std::string("/api/v1/") + ep);
    routes.push_back(std::string("/api/v1/") + ep + "/{{id}}");
    routes.push_back(std::string("/api/v1/") + ep + "/{{id}}/details");
  }

  // A second wide fan-out: 20 action endpoints under /api/v1/admin/
  const char* actions[] = {
    "backup", "restore", "migrate", "audit", "config", "health",
    "metrics", "alerts", "policies", "roles", "permissions", "quotas",
    "billing", "subscriptions", "tenants", "regions", "clusters",
    "deployments", "secrets", "certificates"
  };

  for (auto act : actions) {
    routes.push_back(std::string("/api/v1/admin/") + act);
    routes.push_back(std::string("/api/v1/admin/") + act + "/{{id}}");
  }

  return routes;
}

static std::vector<std::string> generate_wide_lookup_urls(std::mt19937& rng) {
  std::vector<std::string> urls;

  const char* endpoints[] = {
    "users", "posts", "comments", "articles", "products", "orders",
    "invoices", "sessions", "teams", "projects", "tasks", "events",
    "files", "images", "tags", "categories", "settings", "notifications",
    "messages", "logs", "analytics", "reports", "dashboards", "webhooks"
  };

  const char* actions[] = {
    "backup", "restore", "migrate", "audit", "config", "health",
    "metrics", "alerts", "policies", "roles", "permissions", "quotas",
    "billing", "subscriptions", "tenants", "regions", "clusters",
    "deployments", "secrets", "certificates"
  };

  // Hit all 24 endpoints with various IDs
  for (auto ep : endpoints) {
    urls.push_back(std::string("/api/v1/") + ep);
    for (int id = 1; id <= 20; id++) {
      urls.push_back(std::string("/api/v1/") + ep + "/" + std::to_string(id));
      urls.push_back(std::string("/api/v1/") + ep + "/" + std::to_string(id) + "/details");
    }
  }

  // Hit admin actions
  for (auto act : actions) {
    urls.push_back(std::string("/api/v1/admin/") + act);
    for (int id = 1; id <= 10; id++)
      urls.push_back(std::string("/api/v1/admin/") + act + "/" + std::to_string(id));
  }

  std::shuffle(urls.begin(), urls.end(), rng);
  return urls;
}

static std::vector<std::string> generate_wide_miss_urls() {
  return {
    "/api/v1/nonexistent",
    "/api/v1/admin/nonexistent",
    "/api/v2/users",
    "/api/v1/users/123/posts",
    "/api/v1/admin/backup/123/details",
    "/wrong/path",
    "/api/v1/zebra",
  };
}

// ---- Main ----

int main(int argc, char** argv) {
  int iterations = 5;
  if (argc > 1)
    iterations = std::atoi(argv[1]);
  if (iterations < 1)
    iterations = 5;

  const int lookup_multiplier{1000}; // multiply lookups for stable timing

  auto routes = generate_routes();
  std::mt19937 rng(42); // deterministic seed
  auto lookup_urls = generate_lookup_urls(rng);
  auto miss_urls = generate_miss_urls();

  std::printf("===================================\n");
  std::printf("  dynamic_routing_table benchmark\n");
  std::printf("===================================\n");
  std::printf("  Routes registered  : %zu\n", routes.size());
  std::printf("  Lookup URLs (hits) : %zu × %d = %ld\n", lookup_urls.size(),
              lookup_multiplier, (long)lookup_urls.size() * lookup_multiplier);
  std::printf("  Lookup URLs (miss) : %zu × %d = %ld\n", miss_urls.size(),
              lookup_multiplier * 10, (long)miss_urls.size() * lookup_multiplier * 10);
  std::printf("  Iterations         : %d\n", iterations);
  std::printf("===================================\n\n");

  std::vector<double> insert_times, hit_times, miss_times;
  long last_rss = 0;

  for (int i = 0; i < iterations; i++) {
    auto r = run_once(routes, lookup_urls, miss_urls, lookup_multiplier);
    insert_times.push_back(r.insert_ns_per_op);
    hit_times.push_back(r.lookup_hit_ns_per_op);
    miss_times.push_back(r.lookup_miss_ns_per_op);
    last_rss = r.rss_after_insert_kb;
    std::printf("  [iter %d]  insert: %7.1f ns/op  |  hit: %6.1f ns/op  |  miss: %6.1f ns/op\n",
                i + 1, r.insert_ns_per_op, r.lookup_hit_ns_per_op, r.lookup_miss_ns_per_op);
  }

  // Compute medians
  auto median = [](std::vector<double> v) {
    std::sort(v.begin(), v.end());
    size_t n = v.size();
    return (n % 2 == 0) ? (v[n / 2 - 1] + v[n / 2]) / 2.0 : v[n / 2];
  };

  std::printf("===================================\n");
  std::printf("  MEDIAN  insert: %7.1f ns/op  |  hit: %6.1f ns/op  |  miss: %6.1f ns/op\n",
              median(insert_times), median(hit_times), median(miss_times));
  if (last_rss > 0)
    std::printf("  RSS delta after insert: ~%ld KB\n", last_rss);
  std::printf("===================================\n\n");

  // ========== Scenario 2: Wide-node (20+ children per node) ==========
  auto wide_routes = generate_wide_routes();
  std::mt19937 rng2(42);
  auto wide_lookup_urls = generate_wide_lookup_urls(rng2);
  auto wide_miss_urls = generate_wide_miss_urls();

  std::printf("===========================================\n");
  std::printf("  dynamic_routing_table benchmark (WIDE)\n");
  std::printf("===========================================\n");
  std::printf("  Routes registered  : %zu\n", wide_routes.size());
  std::printf("  Lookup URLs (hits) : %zu x %d = %ld\n", wide_lookup_urls.size(),
              lookup_multiplier, (long)wide_lookup_urls.size() * lookup_multiplier);
  std::printf("  Lookup URLs (miss) : %zu x %d = %ld\n", wide_miss_urls.size(),
              lookup_multiplier * 10, (long)wide_miss_urls.size() * lookup_multiplier * 10);
  std::printf("  Iterations         : %d\n", iterations);
  std::printf("===========================================\n\n");

  std::vector<double> w_insert_times, w_hit_times, w_miss_times;
  long w_last_rss = 0;

  for (int i = 0; i < iterations; i++) {
    auto r = run_once(wide_routes, wide_lookup_urls, wide_miss_urls, lookup_multiplier);
    w_insert_times.push_back(r.insert_ns_per_op);
    w_hit_times.push_back(r.lookup_hit_ns_per_op);
    w_miss_times.push_back(r.lookup_miss_ns_per_op);
    w_last_rss = r.rss_after_insert_kb;
    std::printf("  [iter %d]  insert: %7.1f ns/op  |  hit: %6.1f ns/op  |  miss: %6.1f ns/op\n",
                i + 1, r.insert_ns_per_op, r.lookup_hit_ns_per_op, r.lookup_miss_ns_per_op);
  }

  std::printf("===========================================\n");
  std::printf("  MEDIAN  insert: %7.1f ns/op  |  hit: %6.1f ns/op  |  miss: %6.1f ns/op\n",
              median(w_insert_times), median(w_hit_times), median(w_miss_times));
  if (w_last_rss > 0)
    std::printf("  RSS delta after insert: ~%ld KB\n", w_last_rss);
  std::printf("===========================================\n");

  return 0;
}

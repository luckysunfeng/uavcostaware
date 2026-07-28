# Data

`candidate_milp/four_cluster_users_n100_n200.csv` contains the canonical
synthetic user coordinates used for the paired candidate-MILP comparison:

- 30 instances with 100 users, seeds 10000--10029;
- 30 instances with 200 users, seeds 10100--10129;
- 9,000 user records in total;
- columns: `N`, `seed`, `user`, `x`, and `y`.

These are synthetic coordinates in a 1000 m by 1000 m region. They contain no
personal or sensitive information.

The other experiments generate their synthetic instances inside the C
programs from the published seeds and demand-process definitions. The
candidate data are stored explicitly because standard-library C pseudo-random
sequences can differ across runtime libraries.

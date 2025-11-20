# Git Setup Instructions

## Configure Git User (Required before first commit)

Run these commands in your terminal:

```bash
git config user.name "Ihor"
git config user.email "your.email@example.com"
```

Or set globally:
```bash
git config --global user.name "Ihor"
git config --global user.email "your.email@example.com"
```

## Initial Commit

After configuring Git, run:

```bash
cd E:\Project\NYMPH_HandOff_Package_RevA2
git add .
git commit -m "Initial commit: Day 1 - Repository setup and build infrastructure

- Created complete folder structure
- Added build scripts (build.sh, run_local.sh, clean.sh)
- Created README.md
- Set up .gitignore
- Created Docker setup files
- Created Buildroot configuration
- Created all documentation files
- Created hardware device tree templates
- Initialized dist/ with stub manifest"
```

## Note on Permission Issues

If you encounter permission errors when adding files, you may need to:
1. Run Git Bash or PowerShell as Administrator
2. Check folder permissions
3. Ensure no other processes are locking the .git directory


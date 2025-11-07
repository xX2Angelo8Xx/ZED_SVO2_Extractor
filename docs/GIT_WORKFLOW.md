# Git Workflow & Version Control Guide

**Purpose:** Maintain clean, traceable development history  
**Branch Strategy:** Simple trunk-based development with feature branches

---

## 🌳 Branching Strategy

### Main Branches

```
master (main)
  ├── Stable, production-ready code
  └── Protected branch (always buildable)
```

### Feature Branches

```
feature/frame-extractor-cli
feature/video-gui
feature/depth-heatmap
fix/memory-leak-depth
docs/api-reference
```

---

## 📝 Commit Message Format

### Structure:
```
<type>(<scope>): <subject>

<body>

<footer>
```

### Types:
- **feat:** New feature
- **fix:** Bug fix
- **docs:** Documentation changes
- **style:** Code style changes (formatting, no logic change)
- **refactor:** Code restructuring (no feature/bug change)
- **test:** Adding/updating tests
- **chore:** Build process, dependencies, etc.

### Examples:

**Good:**
```
feat(frame-extractor): Add FPS detection and frame skip calculation

- Implement getFPS() method for SVO2 files
- Calculate optimal frame skip for 1fps extraction
- Add unit tests for edge cases (variable framerate)

Closes #12
```

**Good:**
```
fix(metadata): Correct JSON escaping in file paths

Windows paths with backslashes were not properly escaped
in JSON output, causing parse errors.

Fixes #23
```

**Good:**
```
docs(readme): Update build instructions for Visual Studio 2022

Added troubleshooting section for common CMake errors.
```

**Bad:**
```
fixed stuff
```

**Bad:**
```
WIP
```

---

## 🔄 Workflow

### 1. Starting New Work

```powershell
# Always start from updated master
git checkout master
git pull origin master

# Create feature branch
git checkout -b feature/frame-extractor-cli

# Verify you're on correct branch
git branch
# * feature/frame-extractor-cli
#   master
```

### 2. During Development

```powershell
# Check what changed
git status

# Add files to staging
git add src/frame_extractor/main.cpp
git add common/utils.cpp

# Or add all changes
git add .

# Commit with meaningful message
git commit -m "feat(frame-extractor): Implement basic CLI argument parsing

- Add support for --input and --output flags
- Validate folder paths exist
- Display usage help with --help flag"

# Push to GitHub (first time)
git push -u origin feature/frame-extractor-cli

# Push subsequent commits
git push
```

### 3. Committing Frequently

**DO:**
- Commit after completing a logical unit of work
- Commit before taking a break
- Commit before trying something risky

**DON'T:**
- Commit broken code to master
- Make huge commits with multiple unrelated changes
- Forget to commit for days

**Good Commit Frequency:**
```
git log --oneline
abc1234 feat(frame-extractor): Add progress bar display
def5678 feat(frame-extractor): Implement frame numbering logic
ghi9012 feat(frame-extractor): Add PNG export functionality
jkl3456 feat(frame-extractor): Create basic CLI structure
```

### 4. Merging to Master

```powershell
# Update feature branch with latest master
git checkout feature/frame-extractor-cli
git pull origin master

# Test that everything still works
# Build and run tests

# Switch to master
git checkout master
git pull origin master

# Merge feature branch
git merge feature/frame-extractor-cli

# Push to GitHub
git push origin master

# Delete feature branch (optional)
git branch -d feature/frame-extractor-cli
git push origin --delete feature/frame-extractor-cli
```

---

## 🏷️ Tagging Releases

### Create Tag:
```powershell
# Annotated tag (preferred)
git tag -a v0.2.0 -m "Phase 1 complete: Core infrastructure"

# Push tag to GitHub
git push origin v0.2.0

# Push all tags
git push origin --tags
```

### Tag Naming:
- **v0.1.0** - Initial structure
- **v0.2.0** - Core infrastructure complete
- **v0.3.0** - Frame Extractor CLI complete
- **v1.0.0** - First stable release

---

## 🔍 Viewing History

### Recent Commits:
```powershell
# Last 10 commits
git log --oneline -10

# With date and author
git log --pretty=format:"%h - %an, %ar : %s" -10
```

### Changes in Files:
```powershell
# What changed in a file
git diff src/frame_extractor/main.cpp

# Changes between commits
git diff abc1234 def5678

# Changes not yet staged
git diff

# Changes staged for commit
git diff --staged
```

### File History:
```powershell
# See all commits that touched a file
git log --follow src/frame_extractor/main.cpp

# See actual changes
git log -p src/frame_extractor/main.cpp
```

---

## ⏮️ Undoing Changes

### Unstage Files:
```powershell
# Unstage specific file
git restore --staged src/main.cpp

# Unstage all
git restore --staged .
```

### Discard Local Changes:
```powershell
# Discard changes in specific file
git restore src/main.cpp

# Discard all changes (CAREFUL!)
git restore .
```

### Undo Last Commit (Not Pushed):
```powershell
# Keep changes in working directory
git reset --soft HEAD~1

# Discard changes completely (CAREFUL!)
git reset --hard HEAD~1
```

### Revert Published Commit:
```powershell
# Create new commit that undoes changes
git revert abc1234

# This is safe for published commits
```

---

## 🌿 Working with Remotes

### View Remotes:
```powershell
git remote -v
# origin  https://github.com/xX2Angelo8Xx/ZED_SVO2_Extractor.git (fetch)
# origin  https://github.com/xX2Angelo8Xx/ZED_SVO2_Extractor.git (push)
```

### Update from GitHub:
```powershell
# Fetch changes (doesn't modify local files)
git fetch origin

# Pull changes (fetch + merge)
git pull origin master
```

### Push to GitHub:
```powershell
# Push current branch
git push

# Push specific branch
git push origin feature/frame-extractor

# Force push (CAREFUL! Only if you know what you're doing)
git push --force
```

---

## 🚨 Emergency Procedures

### "I committed to wrong branch!"
```powershell
# 1. Note the commit hash
git log --oneline -1
# abc1234 Your commit message

# 2. Switch to correct branch
git checkout correct-branch

# 3. Cherry-pick the commit
git cherry-pick abc1234

# 4. Switch back to wrong branch
git checkout wrong-branch

# 5. Remove the commit
git reset --hard HEAD~1
```

### "I need to go back to previous version"
```powershell
# Find the commit you want
git log --oneline

# Create new branch from that point
git checkout -b recovery abc1234

# Or just view old code temporarily
git checkout abc1234
# (To get back: git checkout master)
```

### "Everything is broken, start over!"
```powershell
# Save current state (just in case)
git stash

# Go back to last good commit
git reset --hard origin/master

# Retrieve stashed changes if needed
git stash pop
```

---

## 📊 Useful Aliases

Add to your git config:

```powershell
git config --global alias.st status
git config --global alias.co checkout
git config --global alias.br branch
git config --global alias.cm commit
git config --global alias.unstage 'restore --staged'
git config --global alias.last 'log -1 HEAD'
git config --global alias.lg "log --graph --pretty=format:'%Cred%h%Creset -%C(yellow)%d%Creset %s %Cgreen(%cr) %C(bold blue)<%an>%Creset' --abbrev-commit"
```

Now you can use:
```powershell
git st        # instead of: git status
git co master # instead of: git checkout master
git lg        # pretty log graph
```

---

## 📅 Daily Workflow Example

### Morning:
```powershell
# Update local repository
cd ZED_SVO2_Extractor
git checkout master
git pull origin master

# Check current status
git status

# See what you were working on
git log --oneline -5
```

### During Work:
```powershell
# Make changes...

# Check what changed
git status
git diff

# Commit logical chunks
git add src/frame_extractor/main.cpp
git commit -m "feat(frame-extractor): Add progress display"

# Continue working...

# Commit again
git add common/utils.cpp common/utils.hpp
git commit -m "refactor(utils): Extract file path validation"
```

### End of Day:
```powershell
# Push all commits
git push

# Check everything is pushed
git status
# Should show: "Your branch is up to date with 'origin/master'"
```

---

## 🔐 Best Practices

### DO:
✅ Commit frequently (small, logical changes)  
✅ Write descriptive commit messages  
✅ Pull before starting work  
✅ Push at end of session  
✅ Create branches for experimental work  
✅ Test before committing to master  
✅ Use tags for milestones  

### DON'T:
❌ Commit broken code to master  
❌ Commit large binary files (use .gitignore)  
❌ Force push to master  
❌ Commit sensitive data (passwords, API keys)  
❌ Make commits with "WIP" or "fix" messages  
❌ Work on master directly for complex features  

---

## 📁 .gitignore

Already configured in project:

```gitignore
# Build files
build/
*.exe
*.obj
*.o

# IDE files
.vs/
.vscode/
*.user

# OS files
Thumbs.db
.DS_Store

# Large output files (configure per your needs)
# output/
# *.mp4
# *.png
```

To ignore additional files:
```powershell
# Edit .gitignore
notepad .gitignore

# Add your patterns
# output/
# *.mp4

# Commit the changes
git add .gitignore
git commit -m "chore: Update gitignore for video output"
```

---

## 🎓 Learning Resources

- **Git Basics:** https://git-scm.com/doc
- **Interactive Tutorial:** https://learngitbranching.js.org/
- **Git Cheat Sheet:** https://education.github.com/git-cheat-sheet-education.pdf
- **Oh Shit, Git!?!** https://ohshitgit.com/ (fixing mistakes)

---

## 📞 Getting Help

```powershell
# Git command help
git help commit
git help merge
git help branch

# Quick command reference
git command --help
```

---

*Practice makes perfect! Don't be afraid to experiment with branches.*

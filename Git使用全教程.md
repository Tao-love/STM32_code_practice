# Git 使用全教程

这是一份适合新手到日常使用的 Git 中文教程。目标是让你能独立完成以下事情：

- 创建仓库
- 保存版本
- 上传到 GitHub
- 拉取远程更新
- 使用分支
- 处理冲突
- 查看历史和回退版本

---

## 1. Git 是什么

Git 是版本控制工具，用来记录文件的变化过程。

它能帮你：

- 看到每次修改了什么
- 误操作后恢复
- 多人协作时减少混乱
- 把代码同步到远程仓库

你可以把它理解成“代码的历史记录系统”。

---

## 2. 三个核心区域

### 2.1 工作区

你正在编辑的文件所在位置。

### 2.2 暂存区

准备提交，但还没正式保存到历史里的内容。

### 2.3 本地仓库

已经通过 `commit` 保存下来的版本历史。

流程可以理解为：

```text
工作区 -> git add -> 暂存区 -> git commit -> 本地仓库 -> git push -> 远程仓库
```

---

## 3. 安装和基础配置

### 3.1 查看 Git 是否安装

```powershell
git --version
```

### 3.2 配置用户名和邮箱

```powershell
git config --global user.name "你的名字"
git config --global user.email "你的邮箱"
```

例如：

```powershell
git config --global user.name "Hu_Tao"
git config --global user.email "33647658@qq.com"
```

查看配置：

```powershell
git config --global --list
```

---

## 4. 创建一个仓库

### 4.1 初始化仓库

进入项目目录后：

```powershell
git init
```

这会生成一个 `.git` 文件夹，说明当前目录已经是 Git 仓库。

### 4.2 查看状态

```powershell
git status
```

常见状态含义：

- `Untracked files`：未被 Git 管理
- `Changes not staged for commit`：已修改但未加入暂存区
- `Changes to be committed`：已加入暂存区，准备提交

---

## 5. 最常用的工作流

日常最常用的三步是：

```powershell
git add .
git commit -m "说明这次修改"
git push
```

含义：

- `git add .`：把改动加入暂存区
- `git commit -m "..."`：把暂存区内容保存成一次提交
- `git push`：把本地提交上传到远程仓库

如果只提交某个文件：

```powershell
git add 文件名
```

---

## 6. 常用命令速查

### 6.1 查看状态

```powershell
git status
```

### 6.2 查看改动

```powershell
git diff
```

查看已暂存的改动：

```powershell
git diff --cached
```

### 6.3 查看提交历史

```powershell
git log
```

简洁显示：

```powershell
git log --oneline --graph --decorate --all
```

### 6.4 查看分支

```powershell
git branch
```

### 6.5 查看远程仓库

```powershell
git remote -v
```

---

## 7. 提交与版本管理

### 7.1 创建提交

```powershell
git add .
git commit -m "修复 OLED 初始化问题"
```

提交信息建议写清楚，避免只写 `update`、`fix` 这种过于笼统的内容。

### 7.2 查看某次提交

```powershell
git show 提交ID
```

例如：

```powershell
git show 1d05881
```

### 7.3 查看最近一次提交

```powershell
git log -1
```

---

## 8. 远程仓库

远程仓库通常是 GitHub、Gitee 或 GitLab。

### 8.1 添加远程仓库

```powershell
git remote add origin https://github.com/用户名/仓库名.git
```

### 8.2 推送到远程仓库

```powershell
git push -u origin master
```

如果你的主分支叫 `main`，就改成：

```powershell
git push -u origin main
```

`-u` 的作用是建立跟踪关系，之后通常可以直接：

```powershell
git push
```

### 8.3 拉取远程更新

```powershell
git pull
```

### 8.4 只下载不合并

```powershell
git fetch
```

---

## 9. 分支基础

分支可以让你在不影响主线代码的情况下开发新功能或修复问题。

### 9.1 查看分支

```powershell
git branch
```

### 9.2 创建分支

```powershell
git branch feature-oled
```

### 9.3 切换分支

```powershell
git switch feature-oled
```

### 9.4 创建并切换到新分支

```powershell
git switch -c feature-oled
```

### 9.5 合并分支

先切回目标分支：

```powershell
git switch master
git merge feature-oled
```

---

## 10. 冲突处理

当两个地方修改了同一段内容时，可能会发生冲突。

Git 会在文件中标记类似下面的内容：

```text
<<<<<<< HEAD
当前分支内容
=======
另一个分支内容
>>>>>>> feature-oled
```

处理方法：

1. 手动编辑文件
2. 保留你需要的内容
3. 删除冲突标记
4. 重新提交

```powershell
git add .
git commit -m "resolve conflict"
```

---

## 11. 回退和撤销

### 11.1 撤销工作区修改

```powershell
git restore 文件名
```

### 11.2 取消暂存

```powershell
git restore --staged 文件名
```

### 11.3 回到某个提交

```powershell
git reset --hard 提交ID
```

这个命令会直接改变当前分支指向，使用前要小心。

### 11.4 更安全的回退方式

如果你已经推送到远程，通常更推荐：

```powershell
git revert 提交ID
```

它会生成一个“反向提交”，用来撤销某次提交的效果。

---

## 12. `.gitignore` 是什么

`.gitignore` 用来告诉 Git 哪些文件不要纳入版本管理。

常见忽略内容：

- 编译产物
- 日志文件
- 临时文件
- 编辑器配置

示例：

```gitignore
.vscode/
*.log
build/
dist/
*.o
*.exe
```

---

## 13. 克隆仓库

把远程仓库复制到本地：

```powershell
git clone 仓库地址
```

例如：

```powershell
git clone https://github.com/Tao-love/STM32_code_practice.git
```

---

## 14. 一个完整示例

假设你要把 `E:\code_guide` 变成 Git 仓库并上传到 GitHub：

```powershell
cd E:\code_guide
git init
git config user.name "Hu_Tao"
git config user.email "33647658@qq.com"
git add .
git commit -m "Initial commit"
git remote add origin https://github.com/Tao-love/STM32_code_practice.git
git push -u origin master
```

之后每次修改并上传，通常只需要：

```powershell
git add .
git commit -m "说明修改"
git push
```

---

## 15. 日常最常用命令

如果你只想记住最核心的一套，记这几个就够了：

```powershell
git status
git add .
git commit -m "说明修改"
git pull
git push
```

---

## 16. 建议

- 提交信息尽量写清楚
- 功能开发尽量使用分支
- 推送前先看 `git status`
- 不熟练时少用 `git reset --hard`


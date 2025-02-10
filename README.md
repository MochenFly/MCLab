# MCLab

## Visual Studio 通用设置

- Visual Studio 2017 Release x64
- 常规
  - Windows SDK：10.0.17763.0
  - 输出目录：$(SolutionDir)Bin
  - 中间目录：$(SolutionDir)Temp
- 调试
  - 命令：\$(SolutionDir)Bin\$(ProjectName)$(TargetExt)
  - 工作目录：\$(SolutionDir)Bin
  - 环境：添加被项目引用的动态库文件所在路径，比如 PATH=\$(QtDllPath);%PATH%;$(SolutionDir)ThirdParty\VTK-9.3.1\bin;
- Qt Project Setting
  - Qt Installation：5.12.12_msvc2017_64
- C++
  - 常规-调试信息格式：程序数据库 (/ZI)
  - 优化-优化：已禁用 (/Od)
- 链接器
  - 调试-生成调试信息：生成调试信息 (/DEBUG)

## 项目目录

### 1. Qt

### 2. VTK

#### 2.1 ImageInteraction

二维图像交互

- 基本交互
- 长方形图像选取
- 椭圆形图像选取
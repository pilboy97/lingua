# Lingua

A tree-walking interpreter for a statically-typed scripting language written in C++.  
静的型付けスクリプト言語のツリーウォーキングインタープリタ（C++実装）。

---

## Features / 機能

- Statically typed with type inference / 静的型付け（型推論あり）
- First-class functions & closures / 第一級関数・クロージャ
- Classes with single inheritance / 単一継承クラス
- Interface types / インタフェース型
- Reference counting GC / 参照カウント方式GC
- `defer` statement / `defer`文
- `is` / `as` type checks and casts / `is`・`as`による型検査とキャスト
- Bitwise, logical, comparison operators / ビット演算・論理演算・比較演算子
- Built-in error handling (`raise` / `Error` interface) / 組み込みエラーハンドリング

---

## Build / ビルド

Requires a C++17-compatible compiler.  
C++17対応コンパイラが必要です。

```bash
g++ -std=c++17 -o lf main.cpp procStruct.cpp
```

---

## Usage / 使い方

```bash
./lingua <source_file.lf>
```

`base.lf` is loaded automatically as the standard library.  
`base.lf` は標準ライブラリとして自動的に読み込まれます。

```bash
# Example / 例
./lingua input.lf
```

---

## Language Syntax / 言語仕様

### Primitive Types / 基本型

| Type / 型 | Description / 説明 |
|---|---|
| `num` | 64-bit integer / 64ビット整数 |
| `real` | 64-bit float / 64ビット浮動小数点 |
| `byte` | 8-bit unsigned integer / 8ビット符号なし整数 |
| `bool` | Boolean / 真偽値 |
| `[]T` | Array of type T / T型の配列 |
| `func(T...) -> R` | Function type / 関数型 |

### Variables / 変数

```lf
var x num = 10
var s = "hello"   // type inferred / 型推論
```

### Functions / 関数

```lf
func add(a num, b num) -> num {
    return a + b
}
```

### Lambda / ラムダ式

```lf
var f = func(x num) -> num { return x * 2 }

// With capture / キャプチャあり
var g = func[captured_var](x num) -> num { return x + captured_var }
```

### Classes / クラス

Fields and methods are separated by `---`.  
フィールドとメソッドは `---` で区切ります。

```lf
class Animal {
    name []byte
    ---
    speak() -> []byte {
        return "..."
    }
}

// Inheritance / 継承
class Dog : Animal {
    ---
    speak() -> []byte {
        return "woof"
    }
}
```

Access modifiers: `+` public (default), `-` private.  
アクセス修飾子：`+` パブリック（デフォルト）、`-` プライベート。

### Interfaces / インタフェース

```lf
interface Error {
    error() -> []byte
}
```

### Control Flow / 制御構文

```lf
// if-else
if (x > 0) {
    ...
} else if (x == 0) {
    ...
} else {
    ...
}

// for (while-style) / for（while形式）
for (i < 10) { ... }

// for (C-style) / for（C言語形式）
for (i = 0; i < 10; i = i + 1) { ... }

// infinite loop / 無限ループ
for { ... }
```

### Type Checks & Casts / 型検査・型変換

```lf
x is num          // type check / 型検査
x is not null     // null check / null検査
x as num          // type cast / 型変換
```

### Defer / 遅延実行

```lf
func example() {
    defer { cleanup() }
    doWork()
}
```

---

## Project Structure / プロジェクト構成

```
/
├── main.cpp            # Entry point / エントリーポイント
├── base.lf             # Standard library / 標準ライブラリ
├── code.hpp            # Token code enum / トークンコード定義
├── token.hpp           # Token struct / トークン構造体
├── parse.hpp           # Lexer / レクサー（字句解析）
├── addScolon.hpp       # Auto semicolon insertion / セミコロン自動挿入
├── keywordDict.hpp     # Keyword & operator table / キーワード・演算子テーブル
├── RDP.hpp             # Recursive descent parser / 再帰下降パーサ
├── procStruct.hpp/.cpp # AST node definitions / ASTノード定義
├── run.hpp             # Tree-walking interpreter / インタープリタ本体
├── panic.hpp           # Error reporting / エラー出力
└── string.hpp          # String utilities / 文字列ユーティリティ
```

### Pipeline / 処理フロー

```
Source (.lf)
  → Lexer / 字句解析 (parse.hpp)
    → Auto semicolon insertion / セミコロン挿入 (addScolon.hpp)
      → Recursive Descent Parser / 再帰下降構文解析 (RDP.hpp)
        → AST
          → Tree-walking Interpreter / インタープリタ実行 (run.hpp)
```

---

## Standard Library / 標準ライブラリ (base.lf)

| Name / 名前 | Description / 説明 |
|---|---|
| `Error` | Base error interface / エラーインタフェース |
| `ErrDivideByZero` | Division by zero error / ゼロ除算エラー |
| `ErrNullPointer` | Null pointer error / nullポインタエラー |
| `makeErrDivideByZero()` | Creates a divide-by-zero error / ゼロ除算エラーを生成 |
| `makeErrNullPointer()` | Creates a null pointer error / nullポインタエラーを生成 |

---

## License / ライセンス

See the `LICENSE` file. / `LICENSE` ファイルを参照してください。

/*
 * scripthighlighter.cpp
 * Copyright 2011, Christophe Conceição <christophe.conceicao@gmail.com>
 *
 * This file is part of Tiled.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "scripthighlighter.h"
#include "mainwindow.h"
#include <QtGui>

ScriptHighlighter::ScriptHighlighter(QTextDocument *parent) : QSyntaxHighlighter(parent)
{
    HighlightingRule rule;

    keywordFormat.setForeground(QColor(64, 64, 255));
    keywordFormat.setFontWeight(QFont::Bold);
    QStringList keywordPatterns = getKeywords();
    foreach (const QString &pattern, keywordPatterns) {
        rule.pattern = QRegExp(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    numberFormat.setForeground(QColor(64, 164, 64));
    numberFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegExp(tr("\\b[0-9]+\\b"));
    rule.format = numberFormat;
    highlightingRules.append(rule);

    quotationFormat.setForeground(QColor(200, 64, 64));
    quotationFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegExp(tr("\".*\""));
    rule.format = quotationFormat;
    highlightingRules.append(rule);
    rule.pattern = QRegExp(tr("\'.*\'"));
    highlightingRules.append(rule);
    rule.pattern = QRegExp(tr("\\[\\[.*\\]\\]"));
    highlightingRules.append(rule);

    functionFormat.setForeground(QColor(128, 128, 255));
    functionFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegExp(tr("\\b[A-Za-z0-9_]+(?=\\()"));
    rule.format = functionFormat;
    highlightingRules.append(rule);

    singleLineCommentFormat.setForeground(QColor(15, 150, 150));
    rule.pattern = QRegExp(tr("--[^\n]*"));
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);

    multiLineCommentFormat.setForeground(QColor(128, 128, 128));
    commentStartExpression = QRegExp(tr("--\\[\\["));
    commentEndExpression = QRegExp(tr(".\\]\\]"));
}

void ScriptHighlighter::highlightBlock(const QString &text)
{
    foreach (const HighlightingRule &rule, highlightingRules) {
        QRegExp expression(rule.pattern);
        int index = expression.indexIn(text);
        while (index >= 0) {
            int length = expression.matchedLength();
            setFormat(index, length, rule.format);
            index = expression.indexIn(text, index + length);
        }
    }
    setCurrentBlockState(0);

    int startIndex = 0;
    if (previousBlockState() != 1)
        startIndex = commentStartExpression.indexIn(text);

    while (startIndex >= 0) {

        int endIndex = commentEndExpression.indexIn(text, startIndex);
        int commentLength;
        if (endIndex == -1) {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        } else {
            commentLength = endIndex - startIndex
                            + commentEndExpression.matchedLength();
        }
        setFormat(startIndex, commentLength, multiLineCommentFormat);
        startIndex = commentStartExpression.indexIn(text, startIndex + commentLength);
    }
}

QStringList ScriptHighlighter::getKeywords()
{
    QStringList keywords;
    keywords << tr("\\bbreak\\b") << tr("\\bdo\\b") << tr("\\bend\\b")
            << tr("\\else\\b") << tr("\\belseif\\b") << tr("\\bfunction\\b")
            << tr("\\bif\\b") << tr("\\blocal\\b") << tr("\\brepeat\\b")
            << tr("\\bnil\\b") << tr("\\bnot\\b") << tr("\\bor\\b")
            << tr("\\breturn\\b") << tr("\\bthen\\b") << tr("\\bwhile\\b")
            << tr("\\buntil\\b") << tr("\\bfor\\b");

    return keywords;
}
